#include "sdk.h"

#include "..\Include\Globals.h"
#include "..\Include\Joysticks.h"
#include "..\Include\Fui.h"
#include "..\Include\FuiUser.h" // for APICreateWindow2
#include "..\Include\MagneticModel.h"
#include "..\Include\Utility.h"

#include <stdio.h>

///////// DEBUG STUFF TO REMOVE LATER ////////////////////////////////
#ifdef  _DEBUG
  //#define _DEBUG_CFuiDLL // print lc DDEBUG file     ... remove later
  //#define _DEBUG_dll2    // print lc DDEBUG file     ... remove later
#endif
//////////////////////////////////////////////////////////////////////

// Search for "//--" to know which APIs are developped up tp now

#ifndef sqr
#define sqr(x)    ((x)*(x))
#endif

SMessage message;

// Please : note that the common used #define lay in the Utility.h header
//#ifndef PI
//#define PI        3.141592653589793
//#endif
//radians scale factor (PI/180)
//#define RADSCALE  0.0174532925199433


void APITestAPISdk(void)
{
  // test 
  #ifdef _DEBUG_dll	
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "Exe 'main' APITestAPISdk %d\n", test);
		  fclose(fp_debug);
	  }
  #endif
}


//
//	object creation, type registration
//

SDLLObject*	APICreateDLLObject(DLLFileRef dll) //
{
  SDLLObject *tmp = NULL;
  //MEMORY_LEAK_MARKER ("APICreateDLLObject");
  if ((tmp = (SDLLObject *) malloc (sizeof (SDLLObject))) == NULL) exit (1);
  //MEMORY_LEAK_MARKER ("APICreateDLLObject");
  return tmp;
}

SDLLObject*	APIInstantiate(unsigned int, unsigned int)
{
  return NULL;
}

SDLLObject*	APICreateScriptObject(DLLFileRef dll, DLLObjectRef ref)
{
  return NULL;
}

void		APIDestroyScriptObject(SDLLObject *object)
{
  return;
}

void		APIAddRegisteredType(SDLLRegisterTypeList **list, EDLLObjectType type, const long signature)
{ //--APIAddRegisteredType
  if (list[1]) {
      list[1]->type = type;
      list[1]->signature = signature;
      //MEMORY_LEAK_MARKER ("APIAddRegisteredType");
      list[1]->next = new SDLLRegisterTypeList;
      //MEMORY_LEAK_MARKER ("APIAddRegisteredType");
      list[2] = list[1]; // bkup = pac
      list[1] = list[1]->next;
      //list[2]->next = NULL;
  }

  #ifdef _DEBUG_dll2	
	  FILE *fp_debug;
    char buf1 [128] = {0};
    TagToString (buf1, type);
    char buf2 [128] = {0};
    TagToString (buf2, signature);
	  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "APIAddRegisteredType %p %s %s\n", list,    buf1, buf2);
      for (int i = 0; i < 3; ++i) {
        char buf3 [128] = {0};
        char buf4 [128] = {0};
        TagToString (buf3, list[i]->type);
        TagToString (buf4, list[i]->signature);
		    fprintf(fp_debug, "%d                    %p %s %s\n", i, list[i], buf3, buf4);
    }
		  fclose(fp_debug);
	  }
  #endif
  return;
}

void		APIRegisterType(EDLLObjectType type, const long signature)
{
  return;
}

void		APIUnregisterType(EDLLObjectType type, const long signature)
{
  return;
}

int		APIGetSDKVersion(void)
{
  return 0;
}

void		APIGetFlyVersion(int *majorRevision, int *minorRevision, int *buildNumber)	// i.e. 2.0.218
{
  return;
}


//
//	messaging primitives
//

void	APISendMessage(SMessage *msg)
{ //-- APISendMessage
  //char buf1 [8] = {0}, buf2 [8] = {0}, buf3 [8] = {0};
  //TagToString (buf1, msg->group);
  //TagToString (buf2, msg->id);
  //TagToString (buf3, msg->user.u.datatag);
  Send_Message (msg);
  //TRACE ("APISendMessage %f %s %s %s", msg->realData, buf1, buf2, buf3);
}


//
//	drawing primitives
//

SSurface*	APICreateSurface(int width, int height)
{ //-- APICreateSurface
  SSurface *surf = CreateSurface (width, height);
  return surf;
}

void	APIFreeSurface(SSurface *surface)
{ //-- APIFreeSurface
  FreeSurface (surface);
  return;
}

void	APIEraseSurface(SSurface *surface)
{ //-- APIEraseSurface
  EraseSurfaceRGB (surface, 0);
  return;
}

void	APIEraseSurfaceRGB(SSurface *surface, unsigned int rgbColor)
{ //-- APIEraseSurfaceRGB
  EraseSurfaceRGBA (surface, rgbColor);
  return;
}

void	APIDrawDot(SSurface *surface, int x, int y, unsigned int rgbColor)
{ //-- APIDrawDot
  DrawDot (surface, x, y, rgbColor);
  return;
}

void	APIDrawLine(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgbColor)
{ //-- APIDrawLine
  DrawFastLine (surface, x1, y1, x2, y2, rgbColor);
  #ifdef _DEBUG_	
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
    {
     fprintf(fp_debug, "APIDrawLine %p %u\n", surface, rgbColor);
     fclose(fp_debug);
    }
  #endif
  return;
}

void	APIDrawRect(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgbColor)
{ //-- APIDrawRect
  DrawRect (surface, x1, y1, x2, y2, rgbColor);
  return;
}

void	APIFillRect(SSurface *surface, int x1, int y1, int x2, int y2, unsigned int rgbColor)
{ //-- APIFillRect
  FillRect (surface, x1, y1, x2, y2, rgbColor);
  return;
}

void	APIDrawCircle(SSurface *surface, int xCenter, int yCenter, int radius, unsigned int rgbColor)
{ //-- APIDrawCircle
  DrawCircle (surface, xCenter, yCenter, radius, rgbColor);
  return;
}

unsigned int	APIMakeRGB(unsigned int r, unsigned int g, unsigned int b)
{ //-- APIMakeRGB
  unsigned int rgb = MakeRGB (r,g,b);
  #ifdef _DEBUG_	
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
    {
	    fprintf(fp_debug, "APIMakeRGB %u %u %u = %u\n", r, g, b, rgb);
	    fclose(fp_debug);
    }
  #endif
  return rgb;
}

void	APIBlit(SSurface *surface)
{ //-- APIBlit
  Blit (surface);
  return;
}

void	APIBlitPartial(SSurface *surface, int x1, int y1, int x2, int y2)
{
  return;
}

void	APIBlitTransparent(SSurface *surface, unsigned int rgbTransparentColor)
{
  return;
}

void	APIBlitPanelToSurface(SSurface *surface, SDLLObject *object)
{
  return;
}

void	APICreateOverlay(SDLLObject *refObject, SSurface *surface, const int x, const int y)
{
  return;
}

void	APIRemoveOverlay(SSurface *surface)
{
  return;
}


//
//	font primitives
//

int	APILoadFont(SFont *font)
{ //-- APILoadFont
  return LoadFont (font);
}

void	APIFreeFont(SFont *font)
{ //-- APIFreeFont
  FreeFont (font);
}

void	APIDrawText(SSurface *surface, SFont *font, int x, int y, unsigned int rgbColor, const char *text)
{ //-- APIDrawText
  //ex : CFont *fnt = (CFont*)globals->fonts.ftthin24.font;
  CFont *fnt = (CFont*) font->font;
  fnt->DrawNText (surface, x, y, rgbColor, (char*) text);
  return;
}

void	APIDrawTextC(SSurface *surface, SFont *font, int x, int y, unsigned int rgbColor, const char *text)
{ //-- APIDrawTextC
  // ex : CFont *fnt = (CFont*)globals->fonts.ftthin24.font;
  CFont *fnt = (CFont*) font->font;
  fnt->DrawTextC (surface, x, y, rgbColor, (char*) text); 
  return;
}

void	APIDrawTextR(SSurface *surface, SFont *font, int x, int y, unsigned int rgbColor, const char *text)
{ //-- APIDrawTextR
  // ex : CFont *fnt = (CFont*)globals->fonts.ftthin24.font;
  CFont *fnt = (CFont*)globals->fonts.ftthin24.font;
  fnt->DrawTextR (surface, x, y, rgbColor, (char*) text);
  return;
}

int	APITextHeight(SFont *font, const char *text)
{
  return 0;
}

int	APITextWidth(SFont *font, const char *text)
{
  return 0;
}

int	APICharHeight(SFont *font, char c)
{
  return 0;
}

int	APICharWidth(SFont *font, char c)
{
  return 0;
}

int	APIMaxCharHeight(SFont *font)
{
  return 0;
}

int	APIMaxCharWidth(SFont *font)
{
  return 0;
}


//
//	bitmap primitives
//

int	APILoadBitmap(SBitmap *bitmap)
{ //-- APILoadBitmap
  return (!Load_Bitmap (bitmap)); // return 0 if success
}

void	APIDrawBitmap(SSurface *surface, SBitmap *bitmap, int x, int y, int frame)
{ //-- APIDrawBitmap
  DrawBitmap (surface, bitmap, x, y, frame);
}

void	APIDrawBitmapPartial(SSurface *surface, SBitmap *bitmap, int dx, int dy, int sx1, int sy1, int sx2, int sy2, int frame)
{ //-- APIDrawBitmapPartial
  DrawBitmapPartial (surface, bitmap, dx, dy, sx1, sy1, sx2, sy2, frame);
}

void	APIGetBitmapSize(SBitmap *bitmap, int *xSize, int *ySize)
{ //-- APIGetBitmapSize
  GetBitmapSize (bitmap, xSize, ySize);
}

int	APINumBitmapFrames(SBitmap *bitmap)
{ //-- APINumBitmapFrames
  return NumBitmapFrames (bitmap);
}

void	APIFreeBitmap(SBitmap *bitmap)
{ //-- APIFreeBitmap
  FreeBitmap (bitmap); // 
}


//
//	stream primitives
//

int	APIOpenStream(SStream *stream)
{
  return 0;
}

void	APICloseStream(SStream *stream)
{
  return;
}

int	APIDoesFileExist(const char *filename, EFileSearchLocation where)
{
  return 0;
}

int	APIDoesPodVolumeExist(const char *volumeName)
{
  return 0;
}

void	APIWriteTo(SDLLObject *object, SStream *stream)
{
  return;
}

void	APIReadFrom(SDLLObject *object, SStream *stream)
{ //-- APIReadFrom
  CStreamObject *str_obj = (CStreamObject *) object->flyObject.objectPtr;
  ReadFrom (str_obj, stream);
}

void	APISkipObject(SStream *stream)
{ //-- APISkipObject
  SkipObject (stream);
}

void	APIAdvanceToTag(unsigned int tag, SStream *stream)
{ //-- APIAdvanceToTag
  AdvanceToTag (tag, stream);
}

void	APIReadInt(int *value, SStream *stream)
{ //-- APIReadInt
  ReadInt (value, stream);
  #ifdef _DEBUG//dll2	
    //TRACE ("APIReadInt %p %d", stream, *value);
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
    {
      fprintf(fp_debug, "APIReadInt %p %d\n", stream, *value);
      fclose(fp_debug);
    }
  #endif
}

void	APIReadUInt(unsigned int *value, SStream *stream)
{ //-- APIReadUInt
  ReadUInt (value, stream);
}

void	APIReadFloat(float *value, SStream *stream)
{ //-- APIReadFloat
  ReadFloat (value, stream);
}

void	APIReadDouble(double *value, SStream *stream)
{ //-- APIReadDouble
  ReadDouble (value, stream);
}

void	APIReadString(char *value, int maxLength, SStream *stream)
{ //-- APIReadString
  ReadString (value, maxLength, stream);
}

void	APIReadVector(SVector *value, SStream *stream)
{ //-- APIReadVector
  ReadVector (value, stream);
}

void	APIReadPosition(SPosition *value, SStream *stream)
{ //-- APIReadPosition
  ReadPosition (value, stream);
}

void	APIReadTime(SDateTime *value, SStream *stream)
{ //-- APIReadTime
  ReadTime (value, stream);
}

void	APIReadTimeDelta(SDateTimeDelta *value, SStream *stream)
{ //-- APIReadTimeDelta
  ReadTimeDelta (value, stream);
}

void	APIReadMessage(SMessage *mesg, SStream *stream)
{ //-- APIReadMessage
  SMessage message;
  if (mesg) {
    memset (&message, 0, sizeof (SMessage));
    ReadMessage (&message, stream);
    if (!message.user.u.datatag) message.user.u.datatag = 'st8t';
    *mesg = message;
    //
    #ifdef _DEBUG_	
      char buf1 [8] = {0}, buf2 [8] = {0};
      TagToString (buf1, message.group);
      TagToString (buf2, message.user.u.datatag);
      TRACE ("APIReadMessage %p %s %s", stream, buf1, buf2);
      FILE *fp_debug;
      if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
      {
        fprintf(fp_debug, "APIReadMessage %p %s %s\n", stream, buf1, buf2);
        fclose(fp_debug);
      }
    #endif
  }
}


void	APIWriteInt(int *value, SStream *stream)
{
  return;
}

void	APIWriteUInt(unsigned int *value, SStream *stream)
{
  return;
}

void	APIWriteFloat(float *value, SStream *stream)
{
  return;
}

void	APIWriteDouble(double *value, SStream *stream)
{
  return;
}

void	APIWriteString(char *value, SStream *stream)
{
  return;
}

void	APIWriteVector(SVector *value, SStream *stream)
{
  return;
}

void	APIWritePosition(SPosition *value, SStream *stream)
{
  return;
}

void	APIWriteTime(SDateTime *value, SStream *stream)
{
  return;
}

void	APIWriteTimeDelta(SDateTimeDelta *value, SStream *stream)
{
  return;
}

void	APIWriteMessage(SMessage *message, SStream *stream)
{
  return;
}


//
//	file primitives
//

int	APIGetFileLength(const char *dir, const char *file)
{
  return 0;
}

int	APIOpenFile(const char *dir, const char *file, SFile *fp)
{
  return 0;
}

void	APICloseFile(SFile *fp)
{
  return;
}

int	APIReadFromFile(SFile *fp, unsigned int numBytes, void *buffer)
{
  return 0;
}


//
//	memory primitives
//

void*	APIAllocMem(unsigned int bytes)
{ //-- APIAllocMem
  //MEMORY_LEAK_MARKER ("APIAllocMem");
  void *pt = malloc (bytes);
  //MEMORY_LEAK_MARKER ("APIAllocMem");
  if (pt) return pt;
  return NULL;
}

void	APIFreeMem(void *address)
{ //-- APIFreeMem
  if (address) free (address);
  return;
}


//
//	sound primitives
//

int	APIPlaySfx(char *wavfile)
{
  return 0;
}

int	APIPlaySfxDelay(char *wavfile, float delay, float *length)
{
  return 0;
}

void	APIStopSfx(int sfxID)				// parameter is value returned from APIPlaySfx
{
  return;
}

void	APIStopSfxByUserData(int user1, int user2)
{
  return;
}

int	APIIsValidSfx(int sfxID)
{
  return 0;
}

void	APISetSfxVolume(int sfxID, float volume)	// volume is 0.0...1.0
{
  return;
}

void	APISetSfxPosition(int sfxID, SPosition *p)
{
  return;
}

void	APISetSfxDirection(int sfxID, SVector *v)
{
  return;
}

void	APISetSfxVelocity(int sfxID, SVector *v)
{
  return;
}

void	APISetSfxFrequency(int sfxID, float freqScalar)	// 1.0 is original rate; 2.0 is 2x rate, etc.
{
  return;
}

void	APISetSfxUserData(int sfxID, int user1, int user2)
{
  return;
}

void	APITextToSpeech(const char *text)
{
  return;
}

int	APITextToSpeechAvailable(void)
{
  return 0;
}


//
//	mouse primitives
//

void	APISetMouseCursor(EMouseCursorStyle style)
{
  return;
}


//
//	keyboard primitives
//

void	APIAddKeyHandler(int keyCode, int modifiers, SDLLObject *object)
{
  return;
}

void	APIRemoveKeyHandler(int keyCode, int modifiers, SDLLObject *object)
{
  return;
}

void	APIDebounceKey(int keyCode, int modifiers)
{
  return;
}

void	APIAddKeyHandler2(int keyID, SDLLObject *object)
{
  return;
}

void	APIRemoveKeyHandler2(int keyID, SDLLObject *object)
{
  return;
}

void	APIDebounceKey2(int keyID)
{
  return;
}

void	APIMimicKeystroke(unsigned int keyID)
{
  return;
}

void	APIDefineKey(int keyID, char *keyFunction, int keyCode, int modifiers)
{
  return;
}

void	APIGetKeyCode(int keyID, int *keyCode, int *modifiers)
{
  return;
}

int	APIGetKeyAssignment(int keyID)
{
  return 0;
}


//
//	OS-independent DLL primitives
//

void*	APILoadLibrary(char *name)
{
  return NULL;
}

void*	APIGetLibraryRoutine(void *library, char *funcName)
{
  return NULL;
}

void	APIFreeLibrary(void *library)
{
  return;
}


//======================================================================
//	joystick functions
//======================================================================

int	APIHasJoystickAxis(EJoystickAxis axis)
{
  //--APIHasJoystickAxis
  switch (axis) {
  case X_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_ELEVATOR ));

  case Y_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_AILERON  ));

  case RUDDER_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_RUDDER   ));

  case TRIM_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_TRIM     ));

  case RBRAKE_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_RITE_TOE));

  case LBRAKE_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_LEFT_TOE));

//  case THROTTLE_AXIS_INDEX :
  case THROTTLE1_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_THROTTLE_1));

  case THROTTLE2_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_THROTTLE_2));
 
  case THROTTLE3_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_THROTTLE_3));

  case THROTTLE4_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_THROTTLE_4));

//  case MIXTURE_AXIS_INDEX :
  case MIXTURE1_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_MIXTURE_1));

  case MIXTURE2_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_MIXTURE_2));

  case MIXTURE3_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_MIXTURE_3));

  case MIXTURE4_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_MIXTURE_4));

//  case PROP_AXIS_INDEX :
  case PROP1_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_PROP_1   ));

  case PROP2_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_PROP_2   ));

  case PROP3_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_PROP_3   ));

  case PROP4_AXIS_INDEX :
    return (globals->jsm->HasAxe(JS_PROP_4   ));

  default :
    return 0;
    break;
  }

  //return 0;
}

float	APIGetJoystickAxis(EJoystickAxis axis)
{ float v = 0;
  //--APIGetJoystickAxis
  switch (axis) {
  case X_AXIS_INDEX :
		globals->jsm->Poll(JS_ELEVATOR,v);
		return v;
  case Y_AXIS_INDEX :
		globals->jsm->Poll(JS_AILERON,v);
		return v;
  case RUDDER_AXIS_INDEX :
		globals->jsm->Poll(JS_RUDDER,v);
		return v;
  case TRIM_AXIS_INDEX :
		globals->jsm->Poll(JS_TRIM,v);
		return v;
  case RBRAKE_AXIS_INDEX :
		globals->jsm->Poll(JS_RITE_TOE,v);
		return v;
  case LBRAKE_AXIS_INDEX :
		globals->jsm->Poll(JS_LEFT_TOE,v);
		return v;
//  case THROTTLE_AXIS_INDEX :
  case THROTTLE1_AXIS_INDEX :
		globals->jsm->Poll(JS_THROTTLE_1,v);
		return v;
  case THROTTLE2_AXIS_INDEX :
		globals->jsm->Poll(JS_THROTTLE_2,v);
		return v;

  case THROTTLE3_AXIS_INDEX :
		globals->jsm->Poll(JS_THROTTLE_3,v);
		return v;
  case THROTTLE4_AXIS_INDEX :
		globals->jsm->Poll(JS_THROTTLE_4,v);
		return v;
//  case MIXTURE_AXIS_INDEX :
  case MIXTURE1_AXIS_INDEX :
		globals->jsm->Poll(JS_MIXTURE_1,v);
		return v;

  case MIXTURE2_AXIS_INDEX :
		globals->jsm->Poll(JS_MIXTURE_2,v);
		return v;
  case MIXTURE3_AXIS_INDEX :
		globals->jsm->Poll(JS_MIXTURE_3,v);
		return v;
  case MIXTURE4_AXIS_INDEX :
		globals->jsm->Poll(JS_MIXTURE_4,v);
		return v;
//  case PROP_AXIS_INDEX :
  case PROP1_AXIS_INDEX :
		globals->jsm->Poll(JS_PROP_1,v);
		return v;
  case PROP2_AXIS_INDEX :
		globals->jsm->Poll(JS_PROP_2,v);
		return v;
  case PROP3_AXIS_INDEX :
		globals->jsm->Poll(JS_PROP_3,v);
		return v;
  case PROP4_AXIS_INDEX :
		globals->jsm->Poll(JS_PROP_4,v);
		return v;
  }

  return v;
}
//----------------------------------------------------------
int	APIGetJoystickButtonCount(void)
{
  return 0;
}

int	APIIsJoystickButtonPressed(int buttonIndex)
{
  return 0;
}


//
//	gauge functions
//

float	APIGetGaugeLightLevel(SDLLObject *object)
{
  return 0.0f;
}

float	APIGetGaugeAmbientLightLevel(SDLLObject *object)
{
  return 0.0f;
}

void	APIHiliteGauge(unsigned int gaugeID, unsigned int hiliteStyle, unsigned int hiliteColor, float timerInSecs)
{
  return;
}

void	APIScrollToGauge(unsigned int gaugeID)
{
  return;
}


//
//	systems functions
//

void	APISetSystemHardwareType(SDLLObject *object, EMessageHWType hwType)
{
  return;
}

EMessageHWType	APIGetSystemHardwareType(SDLLObject *object)
{
  EMessageHWType et = HW_UNKNOWN;
  return et;
}

void	APISetSystemIndication(SDLLObject *object, double indication)
{
  return;
}

double	APIGetSystemIndication(SDLLObject *object)
{
  return 0.0;
}

long	APIGetSystemUnitID(SDLLObject *object)
{
  return 0;
}

int	APIGetSystemUnitNumber(SDLLObject *object)
{
  return 0;
}

void	APISetSystemState(SDLLObject *object, int state)
{
  return;
}

int	APIGetSystemState(SDLLObject *object)
{
  return 0;
}

int	APIIsSystemActive(SDLLObject *object)
{
  return 0;
}

void	APISetSystemLoad(SDLLObject *object, double amps)
{
  return;
}

double	APIGetSystemLoad(SDLLObject *object)
{
  return 0.0;
}

void	APISetSystemMaxVoltage(SDLLObject *object, double voltage)
{
  return;
}

double	APIGetSystemMaxVoltage(SDLLObject *object)
{
  return 0.0;
}


//
//	database functions
//

void	APIGetLocalNavaids(SNavaid **navaids)	 	// caller must call APIFree... when done with list
{
  return;
}

void	APIGetLocalILS(SILS **ils)			// caller must call APIFree... when done with list
{
  return;
}

void	APIGetLocalComms(SComm **comms)		// caller must call APIFree... when done with list
{
  return;
}

void	APIGetLocalCenters(SComm **comms)		// caller must call APIFree... when done with list
{
  return;
}

void	APIGetLocalAirports(SAirport **airports)	// caller must call APIFree... when done with list
{
  return;
}

void	APIGetLocalWaypoints(SWaypoint **waypoints)	// caller must call APIFree... when done with list
{
  return;
}


int	APIGetNearestNavaid(SPosition *pos, int navType, SNavaid *navaid, float *distanceInFeet)
{
  return 0;
}

int	APIGetNearestNavaidByFreq(SPosition *pos, int navType, float freq, SNavaid *navaid, float *distanceInFeet)
{
  return 0;
}

int	APIGetNearestILS(SPosition *pos, float freq, SILS *ils, float *distanceInFeet)
{
  return 0;
}

int	APIGetNearestComm(SPosition *pos, float freq, SComm *comm, float *distanceInFeet)
{
  return 0;
}

int	APIGetNearestAirportComm(SPosition *pos, float freq, SComm *comm, float *distanceInFeet)
{
  return 0;
}

int	APIGetNearestCenterComm(SPosition *pos, float freq, SComm *comm, float *distanceInFeet)
{
  return 0;
}

int	APIGetNearestAirport(SPosition *pos, SAirport *airport, float *distanceInFeet)
{
  return 0;
}

int	APIInAirportRegion(SPosition *pos, float *distanceInFeet)
{
  return 0;
}

int	APIGetAirport(char *airportKey, SAirport *airport)
{
  return 0;
}


int	APISearchNavaidsByID(char *id, int navType, SNavaid **navaids)
{
  return 0;
}

int	APISearchNavaidsByName(char *name, int navType, SNavaid **navaids)
{
  return 0;
}

int	APISearchWaypointsByName(char *name, SWaypoint **waypoints)
{
  return 0;
}

int	APISearchAirportsByFAA(char *faaID, SAirport **airports)
{
  return 0;
}

int	APISearchAirportsByICAO(char *icaoID, SAirport **airports)
{
  return 0;
}

int	APISearchAirportsByName(char *name, SAirport **airports)
{
  return 0;
}

int	APISearchILS(char *airportKey, char *runwayEndID, SILS **ils)
{
  return 0;
}

int	APISearchDatabase(char *db, char *index, char *key, SGeneric **list)
{
  return 0;
}


void	APIFreeNavaid(SNavaid *navaids)	// will free ALL linked navaids
{
  return;
}

void	APIFreeILS(SILS *ils)			// will free ALL linked ils's
{
  return;
}

void	APIFreeWaypoint(SWaypoint *waypoint)	// will free ALL linked waypoints
{
  return;
}

void	APIFreeComm(SComm *comm)	// will free ALL linked comms
{
  return;
}

void	APIFreeAirport(SAirport *airport)	// will free ALL linked airports
{
  return;
}

void	APIFreeGeneric(SGeneric *list)		// will free ALL linked generic records
{
  return;
}


//
//	flight plan functions
//

void	APIGetFlightPlan(SFlightPlan **fp)
{
  return;
}

void	APIFreeFlightPlan(SFlightPlan *fp)
{
  return;
}

void	APIGetFPFirstEntry(SFPEntry *entry)
{
  return;
}

void	APIGetFPLastEntry(SFPEntry *entry)
{
  return;
}

void	APIGetFPToWaypoint(SFPEntry *entry)
{
  return;
}

void	APIGetFPFromWaypoint(SFPEntry *entry)
{
  return;
}

void	APIGetFPBeyondWaypoint(SFPEntry *entry)
{
  return;
}

void	APIUpdateFPCurrentWaypoint(SPosition *pos)
{
  return;
}


//
//	object functions
//
//		note: most of these functions return an int error code (0=no error, 1=invalid object)
//		because any of the objects returned to you from the Fly! sim can disappear at any
//		moment due to the fact that model scenery and dynamic scenery are constantly being
//		added and removed from the environment local to the user aircraft. You can call 
//		APIIsValidObject() at any time to test the validity of the SFlyObject* you have 
//		received from Fly!, but you should check the result codes from each of the following
//		function calls before attempting to use the data returned if the object had gone
//		out of scope
//

SDLLObject*	APICreateFlyObject(char *initFile)
{
  CDLLSimulatedObject *sobj = (CDLLSimulatedObject *) new CDLLSimulatedObject ();
  if (sobj) {
    //globals->sit->dVeh = sobj; // lc 121609
    sobj->sim_objects_active = true;
    SStream s;
    strcpy (s.filename, "WorldObj/");
    strcat (s.filename, initFile);
    strcpy (s.mode, "r");
    if (OpenStream (&s)) {
      ReadFrom (sobj, &s);
      CloseStream (&s);
    }
    SDLLObject *obj = new SDLLObject;
    if (obj) {
      obj->flyObject.objectPtr = sobj;
      obj->flyObject.classSig = sobj->GetTypePointer ();
      obj->flyObject.superSig = NULL;
      globals->sit->sdk_flyobject_list.InsertDLLObjInList (obj);
      #ifdef _DEBUG
        TRACE ("APICreateFlyObject %p %p", sobj, obj);
      #endif
      return obj;
    }
  }
  return NULL;
}

SDLLObject*	APICreateFlyObjectByType(unsigned int objectType)
{
  return NULL;
}


void	APIDestroyObject(SDLLObject *object)
{
#ifdef _DEBUG
  //TRACE ("APIDestroyObject %p %p", object->flyObject.objectPtr, object);
#endif
  //SAFE_DELETE (globals->sit->dVeh); // lc 121609
  SAFE_DELETE (object->flyObject.objectPtr);
  SAFE_DELETE (object);
}

int	APIGetAircraft(SFlyObjectList **objects)		// aircraft only
{
  return 0;
}

int	APIGetObjects(SFlyObjectList **objects)		// all objects (aircraft & models) in the sim
{
  return 0;
}

void	APIFreeObjects(SFlyObjectList *objects)		// use to free allocations made in GetAircraft/GetObjects
{
  return;
}


int	APIGetUserObject(SFlyObjectRef *object)
{
  //--APIGetUserObject
/*!
   initialize the pointer to NULL
   every API will test the NULL before any further operation
   object->objectPtr = NULL;

   NB : - the first element in the fo_list is initialized to NULL
        - every user change (sit->) is responsible for this first
          element pointer modification or NULL
*/
  std::list<SFlyObjectList>::iterator it = globals->sit->sdk_flyobject_list.fo_list.begin ();
  object->objectPtr = (*it).ref.objectPtr;
  object->classSig = globals->pln->GetTypePointer ();
  if (object->objectPtr) return 0;
  return 1;
}

int	APIGetNamedObject(char *name, SFlyObjectRef *object)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIIsValidObject(SFlyObjectRef *object)
{
  //--APIIsValidObject
  if (object->objectPtr) {
    CWorldObject *wo = static_cast<CWorldObject *> (object->objectPtr);
    if (wo) {
//      #ifdef _DEBUG	
//          char buff[16] = {'\0'};
//          TagToString (buff, wo->GetType ());
//	      FILE *fp_debug;
//	      if(!(fp_debug = fopen("__DDEBUG_wobj.txt", "a")) == NULL)
//	      {
//		      fprintf(fp_debug, "APIIsValidObject : %s %d %s\n", buff, wo->GetType (), typeid (wo).name ());
//		      fclose(fp_debug); 
//	      }
//      #endif 
      unsigned int test_type = wo->GetType ();
      switch (test_type) {
        case  TYPE_FLY_WORLDOBJECT :
        case  TYPE_FLY_MODELOBJECT :
        case  TYPE_FLY_SIMULATEDOBJECT :
        case  TYPE_FLY_VEHICLE :
        case  TYPE_FLY_GROUNDVEHICLE :
        case  TYPE_FLY_AIRPLANE :
        case  TYPE_FLY_HELICOPTER :
          return 0;
        default :
          return 1;
      }
    }
    return 1;
  }
  return 1;
}

int	APIGetObjectType(SFlyObjectRef *object, EFlyObjectType *type)
{
  //--APIGetObjectType
  if (object->objectPtr) {
    CWorldObject *wo = static_cast<CWorldObject *> (object->objectPtr);
    if (wo) {
      *type = static_cast<EFlyObjectType> (wo->GetType ());
      return 0;
    }
  }
  return 1;
}


int	APIGetObjectPosition(SFlyObjectRef *object, SPosition *pos)
{
  //--APIGetObjectPosition
  if (object->objectPtr) {
        *pos = static_cast<CDLLSimulatedObject *> (object->objectPtr)->GetPosition();
        return 0;
  }
  return 1;
}

int	APISetObjectPosition(SFlyObjectRef *object, SPosition *pos)
{
  //--APISetObjectPosition
  if (object->objectPtr) {
    // Note : we assume for the moment that object is TYPE_FLY_SIMULATEDOBJECT
    Tag* sig = static_cast<Tag *> (object->classSig);
    switch (*sig) {
      case TYPE_FLY_SIMULATEDOBJECT:
        static_cast<CDLLSimulatedObject *> (object->objectPtr)->SetPosition (*pos);
        return 0;
      default :
      //case TYPE_FLY_VEHICLE:
      //  static_cast<CVehicleObject *> (object->objectPtr)->SetPosition (*pos);
        globals->pln->SetPosition (*pos);
        return 0;
    }
  }
  return 1;
}

int	APIGetObjectOrientation(SFlyObjectRef *object, SVector *orientation)
{
  //--APIGetObjectOrientation [RH]
  if (object->objectPtr) {
    //
    *orientation = static_cast<CWorldObject *> (object->objectPtr)->GetOrientation ();
    return 0;
  }
  return 1;
}

int	APISetObjectOrientation(SFlyObjectRef *object, SVector *orientation)
{
  //--APISetObjectOrientation [RH]
  if (object->objectPtr) {
    // Note : we assume for the moment that object is TYPE_FLY_SIMULATEDOBJECT
    Tag* sig = static_cast<Tag *> (object->classSig);
    switch (*sig) {
      case TYPE_FLY_SIMULATEDOBJECT:
        static_cast<CDLLSimulatedObject *> (object->objectPtr)->SetOrientation (*orientation);
        return 0;
      default:
      //case TYPE_FLY_VEHICLE:
      //  static_cast<CVehicleObject *> (object->objectPtr)->SetOrientation (*orientation);
        globals->pln->SetOrientation (*orientation);
        return 0;
    }
  }
  return 1;
}

int	APIGetObjectBodyVelocity(SFlyObjectRef *object, SVector *velocity)
{
  //--APIGetObjectBodyVelocity [ft/s - RH]
  if (object->objectPtr) {
    CVehicleObject *bv =  static_cast<CVehicleObject *> (object->objectPtr);
    if (bv) {
     SVector tmp_velocity = *(bv->GetBodyVelocityVector ()); // [m/s - LH]
     tmp_velocity = VectorMultiply (tmp_velocity, TC_FEET_PER_METER);
     velocity->x = tmp_velocity.x;
     velocity->y = tmp_velocity.z; // LH->RH
     velocity->z = tmp_velocity.y; // LH->RH
     return 0;
    }
    return 1;
  }
  return 1;
}

int	APISetObjectBodyVelocity(SFlyObjectRef *object, SVector *velocity)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectInertialVelocity(SFlyObjectRef *object, SVector *velocity)
{
 //--APIGetObjectInertialVelocity [ft/s - RH]
 if (object->objectPtr) {
   CVehicleObject *iv = static_cast<CVehicleObject *> (object->objectPtr);
   if (iv ) {
    SVector tmp_velocity = *(iv->GetInertialVelocityVector ()); // [m/s - LH]
    tmp_velocity = VectorMultiply (tmp_velocity, TC_FEET_PER_METER);
    velocity->x = tmp_velocity.x;
    velocity->y = tmp_velocity.z; // LH->RH
    velocity->z = tmp_velocity.y; // LH->RH
    return 0;
   }
   return 1;
 }
 return 1;
}

int	APISetObjectInertialVelocity(SFlyObjectRef *object, SVector *velocity)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectAngularVelocity(SFlyObjectRef *object, SVector *velocity)
{ //--APIGetObjectAngularVelocity [rad/s - RH]
  if (object->objectPtr) {
   CVehicleObject *av = static_cast<CVehicleObject *> (object->objectPtr);
   if (av ) {
    SVector tmp_velocity = *(av->GetBodyAngularVelocityVector ()); // [rad/s - LH]
    velocity->x = tmp_velocity.x;
    velocity->y = tmp_velocity.z; // LH->RH
    velocity->z = tmp_velocity.y; // LH->RH
    return 0;
   }
   return 1;
  }
  return 1;
}

int	APISetObjectAngularVelocity(SFlyObjectRef *object, SVector *velocity)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectSpeed(SFlyObjectRef *object, float *speed)
{
  //--APIGetObjectSpeed   [forward f/s] TAS
  if (object->objectPtr) {
    CVehicleObject *os = static_cast<CVehicleObject *> (object->objectPtr);
    if (os) {
      double tmp = 0.0;
      os->GetTAS (tmp); // in f/s
      *speed = static_cast<float> (tmp);
      return 0;
    }
    return 1;
  }
  return 1;
}
int	APISetObjectSpeed(SFlyObjectRef *object, float *speed)
{
  //--APISetObjectSpeed
  // if (object->objectPtr) { ///< tests are made yet by the called APIs functions
    double distance = static_cast<double> ((*speed) * APIGetDeltaTime ());
    SPosition pos_from, pos_to;
    //pos_from.alt = pos_from.lat = pos_from.lon = pos_to.alt = pos_to.lat = pos_to.lon = 0.0;
    CVector orientation;
    // lc 030707
    if (APIGetObjectPosition    (object, &pos_from)
          ||
        APIGetObjectOrientation (object, &orientation)) return 1;
    // lc 030607
    distance *= cos (orientation.x);
    //
    CVector vect;
    vect.z = 0;
    vect.x = cos (orientation.z + HALF_PI) * distance;
    vect.y = sin (orientation.z + HALF_PI) * distance;
    //
    pos_to = AddVector (pos_from, vect);
    pos_to.alt = pos_from.alt + sin (orientation.x) * distance;
    //
    #ifdef _DEBUG_APISPEED	
        char buff [1024] = {'\0'};
        sprintf (buff, "%.2f %.2f %.2f %.0f %.0f %.0f %.0f %.0f %.2f\n",
                                    orientation.x,
                                    orientation.y,
                                    orientation.z,
                                    pos_from.lat,
                                    pos_from.lon,
                                    pos_to.lat,
                                    pos_to.lon,
                                    pos_to.alt,
                                    distance
                                    );
        APIDrawNoticeToUser (buff, 1);
    #endif

    APISetObjectPosition (object, &pos_to);
    //
    return 0;
  //}
  //return 0;
}

int	APIGetObjectAltitudeRate(SFlyObjectRef *object, float *speed)
{ //--APIGetObjectAltitudeRate ft/s
  if (object->objectPtr) {
    CVehicleObject *av = static_cast<CVehicleObject *> (object->objectPtr);
    if (av ) {
      //
      *speed = MetresToFeet (av->GetInertialVelocityVector ()->y); // LH m/s
      return 0;
    }
  }
  return 1;
}

int	APIGetObjectInertialAcceleration(SFlyObjectRef *object, SVector *accel)
{
 //--APIGetObjectInertialAcceleration [ft.s� - RH]
 if (object->objectPtr) {
   CVehicleObject *av = static_cast<CVehicleObject *> (object->objectPtr);
   if (av ) {
    SVector tmp_velocity = *(av->GetInertialAccelerationVector ()); // [m.s� - LH]
    tmp_velocity = VectorMultiply (tmp_velocity, TC_FEET_PER_METER);
    accel->x = tmp_velocity.x;
    accel->y = tmp_velocity.z; // LH->RH
    accel->z = tmp_velocity.y; // LH->RH
    return 0;
   }
   return 1;
 }
 return 1;
}

int	APIGetObjectBodyAcceleration(SFlyObjectRef *object, SVector *accel)
{
 //--APIGetObjectBodyAcceleration [ft.s� - RH]
 if (object->objectPtr) {
   CVehicleObject *av = static_cast<CVehicleObject *> (object->objectPtr);
   if (av ) {
    SVector tmp_velocity = *(av->GetBodyAccelerationVector ()); // [m.s� - LH]
    tmp_velocity = VectorMultiply (tmp_velocity, TC_FEET_PER_METER);
    accel->x = tmp_velocity.x;
    accel->y = tmp_velocity.z; // LH->RH
    accel->z = tmp_velocity.y; // LH->RH
    return 0;
   }
   return 1;
 }
 return 1;
}

int	APIGetObjectAltitudeAcceleration(SFlyObjectRef *object, float *accel)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectLateralAcceleration(SFlyObjectRef *object, float *accel)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectTrueHeading(SFlyObjectRef *object, float *trueHeadingDegs)
{
  //--APIGetObjectTrueHeading (in reference to meridian lines) -180�/+180�
  /// \todo verify -180�/+180�
  if (!object->objectPtr) {return 1;}
  SVector th;
  float magVarDeg = 0.0f;
  if (!APIGetObjectOrientation       (object, &th)
      &&
      !APIGetObjectMagneticVariation (object, &magVarDeg))
  {
      // invert for right-handed convention : -th.z
      *trueHeadingDegs = Wrap180 (RadToDeg (static_cast<float> (-th.z)) - magVarDeg);
      return 0;
  } else return 1;
}

int	APIGetObjectMagneticHeading(SFlyObjectRef *object, float *magHeadingDegs)
{
  //--APIGetObjectMagneticHeading (magnetic north : offset is magvar) 0�->360�
  /// \todo verify 0�->360�
  if (!object->objectPtr) {return 1;}
  SVector mh;
  if (!APIGetObjectOrientation (object, &mh)) {
    // invert for right-handed convention
    *magHeadingDegs = Wrap360 (RadToDeg (static_cast<float> (-mh.z)));
    return 0;
  } else return 1;
}

int	APIGetObjectMagneticVariation(SFlyObjectRef *object, float *magVarDeg)
{
  //--APIGetObjectMagneticVariation
  /// \todo verify for it seems bugged
  if (object->objectPtr) {
    *magVarDeg = 0.0f;
    SPosition pos;
    if (!APIGetObjectPosition (object, &pos)) {
      float D = 0.0f, 
            H = 0.0f;
      CMagneticModel::Instance().GetElements (pos, D, H);
      *magVarDeg = D;
      return 0;
    }
    return 1;
  }
  return 1;
}

int	APIGetPartPosition(SFlyObjectRef *object, char *partName, SVector *pos)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APISetPartPosition(SFlyObjectRef *object, char *partName, SVector *pos)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetPartOrientation(SFlyObjectRef *object, char *partName, SVector *orientation)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APISetPartOrientation(SFlyObjectRef *object, char *partName, SVector *orientation)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetPartBodyOrientation(SFlyObjectRef *object, char *partName, SVector *bodyPos, SVector *bodyAngle)
{
  if (object->objectPtr) {return 1;}
  return 0;
}

int	APISetPartBodyOrientation(SFlyObjectRef *object, char *partName, SVector *bodyPos, SVector *bodyAngle)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIRotatePart(SFlyObjectRef *object, char *partName, SVector *angle)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIShowPart(SFlyObjectRef *object, char *partName)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIHidePart(SFlyObjectRef *object, char *partName)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APISetPart(SFlyObjectRef *object, char *partName, float status, float pos)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APISetPartFrame(SFlyObjectRef *object, char *partName, int frameIndex)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectName(SFlyObjectRef *object, char *name, int maxLength)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APISetObjectName(SFlyObjectRef *object, char *name)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetTailNumber(SFlyObjectRef *object, char *name, int maxNameLength, char *abbrName, int maxAbbrLength)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APISetTailNumber(SFlyObjectRef *object, char *name, char *abbreviatedName)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectAGL(SFlyObjectRef *object, float *agl)
{
  //--APIGetObjectAGL
  if (object->objectPtr) {
    CVehicleObject *vo = static_cast<CVehicleObject *> (object->objectPtr);
    if (vo) {*agl = vo->GetUserAGL (); return 0;}
    return 1;
  }
  return 1;
}

int	APIGetObjectMSL(SFlyObjectRef *object, float *msl)
{
  //--APIGetObjectMSL
  if (object->objectPtr) {
    CVehicleObject *vo = static_cast<CVehicleObject *> (object->objectPtr);
    if (vo) {*msl = vo->GetUserAGL () + globals->tcm->GetGroundAltitude(); return 0;}
    return 1;
  }
  return 1;
}

int	APIGetObjectBodyGravity(SFlyObjectRef *object, SVector *gravity)
{
  //--APIGetObjectBodyGravity
  /// \todo verify and test to validate
  if (object->objectPtr) {
    float weight = 0.0f;
    if (!APIGetObjectTotalWeight (object, &weight)) {
      CVector vertical_gravity;
      vertical_gravity.x = 0.0;
      vertical_gravity.z = 0.0;
      vertical_gravity.y = static_cast<double> (-weight);                 // LH
      SVector ori;
      //
      if (!APIGetObjectOrientation (object, &ori)) {                       // RH
      //*/
      CRotationMatrixHPB matx;                                            // LH
      VectorOrientLeftToRight  (ori);                                     // RH->LH
      matx.Setup (ori);                                                   // LH
      SVector tmp;
      matx.ParentToChild (tmp, vertical_gravity);                         // LH
      gravity->x = tmp.x;                                                 // LH->RH
      gravity->y = tmp.z;                                                 // LH->RH
      gravity->z = tmp.y;                                                 // LH->RH
      //*/
        return 0;
      } else {
        return 1;
      }
    } else {
      return 1;
    }
  }
  return 1;
}

int	APIGetObjectTotalWeight(SFlyObjectRef *object, float *weight)
{
 //--APIGetObjectTotalWeight
 if (object->objectPtr) {
   CVehicleObject *vo = static_cast<CVehicleObject *> (object->objectPtr);
   if (vo) {
     // note that GetTotalMassInLbs actually returns a weight in lbs
     *weight = vo->wgh->GetTotalMassInLbs ();
     return 0;
   }
   return 1;
 }
  return 1;
}

int	APIGetObjectPressureAltitude(SFlyObjectRef *object, float *altitude)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectDensityAltitude(SFlyObjectRef *object, float *altitude)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectInertialGroundSpeed(SFlyObjectRef *object, SVector *speed)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectBodyGroundSpeed(SFlyObjectRef *object, SVector *speed)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetObjectGroundSpeed(SFlyObjectRef *object, float *speed)
{
  //--APIGetObjectGroundSpeed
  if (object->objectPtr) {
    CVehicleObject *os = static_cast<CVehicleObject *> (object->objectPtr);
    if (os) {
      *speed = os->GetGSpeed ();
      return 0;
    }
    return 1;
  }
  return 1;
}

int	APIGetInertialWindVector(SFlyObjectRef *object, SVector *vector)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetBodyWindVector(SFlyObjectRef *object, SVector *vector)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetWindSpeed(SFlyObjectRef *object, float *speed)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetWindHeading(SFlyObjectRef *object, float *heading)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetHeadWind(SFlyObjectRef *object, float *headWind)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetTailWind(SFlyObjectRef *object, float *tailWind)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetOutsideAirTemperature(SFlyObjectRef *object, float *tempInF)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetOutsideAirPressure(SFlyObjectRef *object, float *inHg)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetTotalAirTemperature(SFlyObjectRef *object, float *tempInF)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetMachNumber(SFlyObjectRef *object, float *mach)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetModelRadius(SFlyObjectRef *object, float *radius)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIIsObjectNearGround(SFlyObjectRef *object, int *trueFalse)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIAreWheelsOnGround(SFlyObjectRef *object, int *wheelsOnGround)
{
  //--APIAreWheelsOnGround
  if (object->objectPtr) {
    CVehicleObject *vo = static_cast<CVehicleObject *> (object->objectPtr);
    if (vo) {
      int w_num = vo->whl->GetNumberOfWheels ();                  ///< number of body's wheels
      if (w_num == 0) {return 1;}                                 ///< no wheels at all !
      int num   = vo->whl->GetNbWheelOnGround ();                 ///< number of wheels on ground
      //
      if   (num   == 0)     {*wheelsOnGround = 0; return 0;}      ///< no wheels on ground
      else {
        /// \todo wheels in water : link with ground type
        if (vo->GetTerrainType () == TERRAIN_WATER_OCEAN) {       ///< wheels in water
                             *wheelsOnGround = 4; return 0;
        } else {                                                  ///< wheels on ground :
          if (num   == 1)   {*wheelsOnGround = 3; return 0;}      ///< one wheel on ground
          else
          if (num == w_num) {*wheelsOnGround = 2; return 0;}      ///< all wheels on ground
          else              {*wheelsOnGround = 1; return 0;}      ///< some wheels on ground
        }
      }
    } else return 1;
  } else return 1;
}

int	APIGetGroundTypeUnderObject(SFlyObjectRef *object, int *groundType)
{
  /// \todo change GetGroundType
  //-- APIGetGroundTypeUnderObject
  if (object->objectPtr) {
    CVehicleObject *vo = static_cast<CVehicleObject *> (object->objectPtr);
    if (vo) {
      ETerrainType terrain_type = vo->GetTerrainType ();
      switch (terrain_type) {
        case TERRAIN_WATER_OCEAN:
          *groundType = static_cast<int> (1);
          break;
        default :
          return 1;
      }
      return 0;
    }
    return 1;
  }
  return 1;
}

int	APIIsGearUp(SFlyObjectRef *object, int *gearUp)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIIsGearDown(SFlyObjectRef *object, int *gearDown)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIAreBrakesOn(SFlyObjectRef *object, int *brakesOn)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APITuneRadio(SFlyObjectRef *object, ERadioType type, int radioUnit, float freq)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APITuneRadio2(SFlyObjectRef *object, ERadioType type, int radioUnit, float freq, EFreqType whichFreq)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

float	APIGetTunedFreq(SFlyObjectRef *object, ERadioType type, int radioUnit, EFreqType whichFreq)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetTunedType(SFlyObjectRef *object, ERadioType type, int radioUnit)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetTunedNavaid(SFlyObjectRef *object, ERadioType type, int radioUnit, SNavaid *navaid)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetTunedILS(SFlyObjectRef *object, ERadioType type, int radioUnit, SILS *localizer, SILS *glideslope, SILS *outer, SILS *middle, SILS *inner, SILS *backcourse)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIGetTunedComm(SFlyObjectRef *object, ERadioType type, int radioUnit, SComm *comm)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIObjectInRegion(SFlyObjectRef *object, SPosition *ll, SPosition *ur, int *trueFalse)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIObjectInAirport(SFlyObjectRef *object, SAirport *airport, int *trueFalse)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APISimplifyObject(SFlyObjectRef *object)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APILoadModel(SFlyObjectRef *object, const char *modelName, const unsigned int modelID)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APILoadNightModel(SFlyObjectRef *object, const char *modelName, const unsigned int modelID)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APISelectModel(SFlyObjectRef *object, const unsigned int modelID)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APISelectNightModel(SFlyObjectRef *object, const unsigned int modelID)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIRayWorldIntersection(SPosition *p1, SPosition *p2, SFlyObjectRef *avoid, SCollision *collision, int collideFlags)
{
  if (avoid->objectPtr) {return 0;}
  return 1;
}

int	APIGetEngineState(SFlyObjectRef *object, int engineNum)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

int	APIAllEnginesRunning(SFlyObjectRef *object)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

void	APITurnOffAllAutopilots(SFlyObjectRef *object)
{
  return;
}

void	APILockoutUserControl(SFlyObjectRef *object)
{
  return;
}


SFlyObjectRef	APICreateVehicle(char *infoFile, char *pathFile, SPosition *pos)
{
  SFlyObjectRef ref;
  ref.classSig = NULL;
  ref.objectPtr = NULL;
  ref.superSig = NULL;
  return ref;
}

void	APIDisposeVehicle(SFlyObjectRef *object)
{
  return;
}

int	APISetVehiclePath(SFlyObjectRef *object, char *pathFile)
{
  if (object->objectPtr) {return 0;}
  return 1;
}

void	APIAddGeneratedScenery(SPosition *pos, SVector *orient, char *daySmfFile, char *nightSmfFile, int autoHide)
{
  return;
}


//
//	gestalt functions (object traits)
//

int	APIGestalt(SFlyObjectRef *object, int gestaltFlag)
{
  return 0;
}

void	APIGestaltSet(SFlyObjectRef *object, int gestaltFlag)
{
  return;
}

void	APIGestaltClear(SFlyObjectRef *object, int gestaltFlag)
{
  return;
}


//
//	position and reference utilities
//

int	APIPositionFromNamedObject(const char *objectName, SPosition *pos)
{
  return 0;
}

int	APIDistanceToNamedObject(const char *objectName, SPosition *pos, float *distance)
{
  return 0;
}

SPosition	APIMakePosition(const char *latString, const char *lonString, const float alt)
{
  SPosition pos;
  pos.alt = 0.0;
  pos.lat = 0.0;
  pos.lon = 0.0;
  return pos;
}

int	APIPointInRegion(SPosition *pos, SPositionList *region)
{
  return 0;
}

int	APIPointInCircle(SPosition *pos, SPosition *center, float radiusInFeet)
{
  return 0;
}

int	APIPointInAirport(SPosition *pos, SAirport *airport)
{
  return 0;
}

void	APICalculateRunwayRegions(SRunway *runway, SPositionList **upwind, SPositionList **crosswind, SPositionList **downwind, SPositionList **base, SPositionList **final)
{
  return;
}

void	APIDisposePositionList(SPositionList *region)
{
  return;
}


//
//	ground functions
//

float	APIGetGroundHeight(SPosition *pos)
{
 //--APIGetGroundHeight
  return 0;
}

SVector	APIGetGroundNormal(SPosition *pos)
{ 
  //--APIGetGroundNormal
  CVector tmp;
//  return CTerrainManager::Instance ().GetGroundNormal (*pos);
  return tmp;
}

int	APIGetGroundType(SPosition *pos)
{
  /// \todo change GetGroundType
  //--APIGetGroundType
  return TERRAIN_WATER_OCEAN;
}

int	APIDoesSceneryExist(SPosition *ll, SPosition *ur)
{
  return 0;
}


//
//	checklist functions
//

int	APIIsChecklistRunning(void)
{
  return 0;
}

void	APIRunChecklist(const char *checklist)
{
  return;
}


//
//	weather functions
//

void	APILoadMetar(char *filePath)
{
  return;
}

void	APIGetWeatherInfo(SWeatherInfo *info)
{
  return;
}

void	APIGetWindsAloft(SWindsAloft winds[WEATHER_WINDS_ALOFT_MAX])
{
  return;
}

void	APISetWeatherInfo(SWeatherInfo *info)
{
  return;
}

void	APISetWindsAloft(SWindsAloft winds[WEATHER_WINDS_ALOFT_MAX])
{
  return;
}

void	APIGetReportedWeather(SPosition *pos, SWeatherInfo *info)
{
  return;
}

void	APISetFogColor(int r, int g, int b)
{
  return;
}

void	APISetBrokenCloudTextures(char textures[3][64])
{
  return;
}

void	APISetOvercastCloudTexture(char texture[64])
{
  return;
}

void	APISetCloudPanelTextures(char texture[3][64])
{
  return;
}

void	APISetLightningTextures(char texture[3][64])
{
  return;
}

void	APISetRainTextures(char texture[3][64])
{
  return;
}

void	APISetSnowTexture(char texture[64])
{
  return;
}

void	APISetSkyBackdrop(char texture[64])						// obsolete
{
  return;
}

void	APIShowSkyBackdrop(char trueFalse)						// obsolete
{
  return;
}

void	APISetDomeRGBSaturation(float redSat, float greenSat, float blueSat)		// obsolete
{
  return;
}

void	APIGetDomeRGBSaturation(float *redSat, float *greenSat, float *blueSat) 	// obsolete
{
  return;
}

float	APIGetAmbientLightLevel(void)
{
  return 0.0f;
}

void	APISetAmbientLightMultiplier(float multiplier)
{
  return;
}

float	APIGetAmbientLightMultiplier(void)
{
  return 0.0f;
}

int	APIVisibleMoisture(int *level)
{
  return 0;
}

void	APIGetLightScalars(float *dawn, float *day, float *dusk, float *night)
{
  return;
}

void	APISetLightScalars(float dawn, float day, float dusk, float night)
{
  return;
}


//
//	sky functions
//

void	APILoadSkyModels(const char *skyIniFilename)
{
  return;
}

void	APISelectSkyModel(const char *skyEntryName)
{
  return;
}

int	APIRandomlySwapSkies(int flag)	// flag=0 no swapping, flag=1 randomly swaps at dawn; returns previous setting
{
  return 0;
}


//==================================================================
//	camera functions
//==================================================================

void	APISetCamera(unsigned int camera)
{
  //--APISetCamera
  CCameraManager *ccm = globals->ccm;
  if (ccm) ccm->SelectCamera (camera);
}

unsigned int	APIGetCamera(void)
{
  //--APIGetCamera
  return globals->cam->GetCameraType ();
}

int	APIHasCamera(SFlyObjectRef *ref, unsigned int camera)
{
  return 0;
}

void	APIAddCamera(SFlyObjectRef *ref, unsigned int camera, SStream *stream)
{
  return;
}

void	APISetPanel(const int panelID)
{
  return;
}

void	APISetToRightPanel(void)
{
  return;
}

void	APISetToLeftPanel(void)
{
  return;
}

void	APISetToUpperPanel(void)
{
  return;
}

void	APISetToLowerPanel(void)
{
  return;
}

void	APIHomePanel(void)
{
  return;
}

void	APIShowPanel(void)
{
  return;
}

void	APIHidePanel(void)
{
  return;
}

int	APIIsInteriorCamera(void)
{
  //--APIIsInteriorCamera
  return globals->inside;
}

int	APIIsExteriorCamera(void)
{
  //--APIIsExteriorCamera
  return 1 ^ globals->inside;
}

void	APIPanCamera(float pitchInRads, float headingInRads, int autoCenter)
{
  return;
}

void	APIPositionCamera(float pitchInRads, float headingInRads, float distanceInFeet)
{
  //--(APIPositionCamera)
  if (APIIsExteriorCamera ()) {
    CCamera *cam = globals->cam;
    cam->SetCameraPosition (pitchInRads, headingInRads, distanceInFeet);
  }
}

void	APIZoomCamera(float zoomRatio)
{
  //--(APIZoomCamera
  if (APIIsExteriorCamera ()) {
    CCamera *cam = globals->cam;
    cam->SetZoom(zoomRatio);
  }
}

void	APIUseVFRPanels(void)
{
  return;
}

void	APIUseIFRPanels(void)
{
  return;
}

void	APIGetCameraPosition(SPosition *pos, SVector *dir, SVector *vel)
{
  //--(APIGetCameraPosition) --> still to finish for vel
  if (APIIsExteriorCamera ()) {
    CCameraManager *ccm = globals->ccm;
    if (ccm) {
      SPosition u_pos = globals->geop;
      SVector u_ori   = globals->iang;
      float dT = APIGetDeltaTime ();
      ccm->UpdateCamera (u_pos, u_ori, dT);
      /// \todo vel
      vel->x = 0.0; // 
      vel->y = 0.0; // 
      vel->z = 0.0; // 
    }
  }
}


//
//	window functions
//

SDLLObject*	APICreateWindow(int type, int x, int y, int xSize, int ySize, int windowFlags)
{
  return NULL;
}

void	APIWindowSetAutoSize(SDLLObject *object, int onOffFlag)
{
  return;
}

void	APIWindowSetMoveable(SDLLObject *object, int onOffFlag)
{
  return;
}

void	APIWindowSetResizeable(SDLLObject *object, int onOffFlag)
{
  return;
}

void	APIWindowSetDrawTitleBar(SDLLObject *object, int onOffFlag)
{
  return;
}

void	APIWindowSetDrawCloseBox(SDLLObject *object, int onOffFlag)
{
  return;
}

void	APIWindowSetDrawSizeBox(SDLLObject *object, int onOffFlag)
{
  return;
}

void	APIWindowSetDrawBorder(SDLLObject *object, int onOffFlag)
{
  return;
}

void	APIWindowSetMinSize(SDLLObject *object, int minXSize, int minYSize)
{
  return;
}

void	APIWindowSetMaxSize(SDLLObject *object, int maxXSize, int maxYSize)
{
  return;
}

void	APIWindowSetPosition(SDLLObject *object, int x, int y)
{
  return;
}

void	APIWindowSetSize(SDLLObject *object, int xSize, int ySize)
{
  return;
}

void	APICloseWindow(SDLLObject *object)
{
  return;
}

void	APICloseAllWindows(void)
{
  return;
}


void	APICreateWindowByType(int type, int x, int y, int xSize, int ySize)
{
  return;
}

void	APIMoveWindowByType(int type, int x, int y)
{
  return;
}

void	APISizeWindowByType(int type, int x, int y)
{
  return;
}

void	APICloseWindowByType(int type)
{
  return;
}

int	APIWindowCount(int type)
{
  return 0;
}


void	APICreateAirportWindow(SAirport* airport, int x, int y)
{
  return;
}

void	APICloseAirportWindow(SAirport* airport)
{
  return;
}

void	APICreateNavaidWindow(SNavaid* navaid, int x, int y)
{
  return;
}

void	APICloseNavaidWindow(SNavaid* navaid)
{
  return;
}


void	APIVectorMapDrawAirports(const int state)
{
  return;
}

void	APIVectorMapDrawVOR(const int state)
{
  return;
}

void	APIVectorMapDrawNDB(const int state)
{
  return;
}

void	APIVectorMapDrawAirportName(const int state)
{
  return;
}

void	APIVectorMapDrawAirportID(const int state)
{
  return;
}

void	APIVectorMapDrawVORName(const int state)
{
  return;
}

void	APIVectorMapDrawVORID(const int state)
{
  return;
}

void	APIVectorMapDrawVORFrequency(const int state)
{
  return;
}

void	APIVectorMapDrawNDBName(const int state)
{
  return;
}

void	APIVectorMapDrawNDBID(const int state)
{
  return;
}

void	APIVectorMapDrawLabels(const int state)
{
  return;
}

void	APIVectorMapDrawCompassPlate(const int state)
{
  return;
}

void	APIVectorMapDrawBackground(const int state)
{
  return;
}

void	APIVectorMapZoom(const float zoomPercent)
{
  return;
}


//
//	damage functions
//

int	APIUserNeedsRepair(void)
{
  return 0;
}

void	APIRepairUser(void)
{
  return;
}

float	APIReportAllDamage(void)
{
  return 0.0f;
}

float	APIReportRotorDamage(void)
{
  return 0.0f;
}

float	APIReportPropellerDamage(void)
{
  return 0.0f;
}

float	APIReportEngineDamage(void)
{
  return 0.0f;
}

float	APIReportWingDamage(void)
{
  return 0.0f;
}

float	APIReportWheelDamage(void)
{
  return 0.0f;
}


//
//	date and time functions
//

SDate	APIGetDate(void)
{
  //--APIGetDate
  SDateTime dt = globals->tim->GetUTCDateTime ();
  return dt.date;
}

void	APISetDate(SDate date)
{
  //--APISetDate
  SDateTime dt = globals->tim->GetUTCDateTime ();
  dt.date = date;
  globals->tim->SetUTCDateTime (dt);
}

STime	APIGetTime(void)
{
  //--APIGetTime
  SDateTime dt = globals->tim->GetUTCDateTime ();
  return dt.time;
}

void	APISetTime(STime time)
{
  //--APISetTime
  SDateTime dt = globals->tim->GetUTCDateTime ();
  dt.time = time;
  ////
  //#ifdef _DEBUG	
  //  DEBUGLOG ("APISetTime %d-%d-%d %d-%d-%d\n", 
  //             dt.date.day, dt.date.month, dt.date.year,
  //             dt.time.hour, dt.time.minute, dt.time.second);
  //#endif 
  globals->tim->SetUTCDateTime (dt);
}

ETimeOfDay	APIGetTimeOfDay(void)
{
  ETimeOfDay ed = TIME_DAYTIME;
  return ed;
}

void	APIAdvanceTime(STime *time, float secs)
{
  //--APIAdvanceTime --> to be finished
  SDateTimeDelta delta;
  delta.dYears = delta.dMonths = delta.dDays = delta.dHours = 0;
  delta.dMinutes = 0;
  delta.dSeconds = static_cast<unsigned int> (time->second + secs);
  delta.dMillisecs = 0;

  SDateTime dt = globals->tim->GetUTCDateTime ();
  dt = globals->tim->AddTimeDelta (dt, delta);
  globals->tim->SetUTCDateTime (dt);
}

float	APIGetDeltaTime(void)
{
  //--APIGetDeltaTime
  return (globals->tim->GetDeltaSimTime ());
}

unsigned int	APIGetSystemTimer(void)
{
  return 0;
}


//
//	movie functions
//

SMovie*	APIOpenMovie(const char *movFile)
{
  return NULL;
}

void	APIDrawMovie(SMovie *movie, int screenX, int screenY)
{
  return;
}

void	APIPlayMovie(SMovie *movie)
{
  return;
}

void	APIStopMovie(SMovie *movie)
{
  return;
}

void	APIIdleMovie(SMovie *movie)
{
  return;
}

int	APIMoviePlaying(SMovie *movie)
{
  return 0;
}

void	APICloseMovie(SMovie *movie)
{
  return;
}

void	APIPlayMovieInWindow(const char *movFile, int screenX, int screenY, int autoClose)
{
  return;
}

void	APIBeginRecordMovie(char *movFile, const int xSize, const int ySize, const int fps)
{
  return;
}

void	APIIdleRecordMovie(void)
{
  return;
}

void	APIEndRecordMovie(void)
{
  return;
}

int	APIIsRecordingMovie(void)
{
  return 0;
}

int	APIMovieFramesPerSecond(void)
{
  return 0;
}


//
//	general info functions
//

int	APIIsSimulationRunning(void)
{
  //--APIIsSimulationRunning
  /// \todo : there should be a better way
  // globals->appState = APP_SIMULATION; ???
  if (APIIsPaused () || APIIsSlewing ()) return 0;
  return 1;
}

int	APIIsSlewing(void)
{
  //--APIIsSlewing
  if (globals->slw->IsEnabled ()) return 1;
  return 0;
}

int	APIIsPaused(void)
{
  //--APIIsPaused
  if (globals->tim->GetPauseState ()) return 1;
  return 0;
}

void	APISetSlewing(const int slewFlag)
{
  //--APISetSlewing
  globals->slw->StateAs(slewFlag);
}

void	APISetPaused(const int pauseFlag)
{
  //--APISetPaused
  globals->tim->Pause ();
}

void	APIGetMainWindowCoords(int *x1, int *y1, int *x2, int *y2)
{ //-- APIGetMainWindowCoords
  *x1 = globals->mScreen.X;
  *y1 = globals->mScreen.Y;
  *x2 = *x1 + globals->mScreen.Width;
  *y2 = *y1 + globals->mScreen.Height;
  return;
}

void	APIGetScreenResolution(int *xSize, int *ySize)
{ //-- APIGetScreenResolution
  *xSize = globals->mScreen.Width;
  *ySize = globals->mScreen.Height;
  return;
}

void	APIGetScreenDirect(
			unsigned char **rowTable[], 
			int *xSize, int *ySize, int *bpp, 
			int *redScale, int *redShift,
			int *greenScale, int *greenShift,
			int *blueScale, int *blueShift,
			int *byteOrder)
{
  return;
}


//
//	feedback functions
//

void APIDrawNoticeToUser(const char *text, const int timeoutInSecs)
{
  globals->fui->DrawNoticeToUser ((char*)text, static_cast<float>(timeoutInSecs));
}


void	APIMessageBox(const char *message)
{
  //--APIDrawNoticeToUser
  //  MessageBoxToUser (message);

  // Create
  globals->fui->CreateFuiWindow  (FUI_WINDOW_ALERT_MSG);
  globals->fui->SetComponentText (FUI_WINDOW_ALERT_MSG, 'mesg', (char*)message);
}

//
//	utility functions
//

float	APIGreatCircleDistance(SPosition *from, SPosition *to)
{
  //--APIGreatCircleDistance
  //  return (GreatCircleDistance (from, to));
  //  workaround until GreatCircleDistance is ready
  //  float lat1, float lon1, float lat2, float lon2
    return (static_cast<float>(2 * asin( sqrt(  
                    sqr( (sin( (from->lat-to->lat)/2 ) ) ) + 
                    cos(from->lat) * cos(to->lat) * sqr( (sin( (from->lon-to->lon)/2 ))  )  
                    ) )) );

}

SPosition	APIGreatCirclePosition(SPosition *from, SVector *polar)
{
  //--APIGreatCirclePosition
  return (GreatCirclePosition (from, polar));
}

SVector	APIGreatCirclePolar(SPosition *from, SPosition *to)
{
  //--APIGreatCirclePolar
  return (GreatCirclePolar (from, to));
}

float	APIWrap180(float value)
{
  //--APIWrap180
  return (Wrap180 (value));
}

float	APIWrap360(float value)
{
  //--APIWrap360
  return (Wrap360 (value));
}

float	APIWrapPi(float value)
{
  //--APIWrapPi
  return (WrapPI (value));
}

float	APIWrapTwoPi(float value)
{
  //--APIWrapTwoPi
  return (WrapTwoPi (value));
}

SPosition	APIAddVector(SPosition *from, SVector *v)
{
  //--APIAddVector
  return (AddVector(*from, *v));
}

SPosition	APISubtractVector(SPosition *from, SVector *v)
{
  //--APISubtractVector
  return (SubtractVector(*from, *v));
}

SVector	APISubtractPosition(SPosition *from, SPosition *to)
{
  //--APISubtractPosition
  return (SubtractPosition(*from, *to));
}


//
//	ini functions
//

void	APIGetIniVar(const char *section, const char *varname, int *value)
{
  //-- APIGetIniVar
  int tmp = 0;
  GetIniVar (section, varname, &tmp);
  *value = tmp;
}

void	APIGetIniString(const char *section, const char *varname, char *strvar, int maxLength)
{
  //-- APIGetIniString
  GetIniString (section, varname, strvar, maxLength);
}

void	APISetIniVar(const char *section, const char *varname, int *value)
{
  return;
}

void	APISetIniString(const char *section, const char *varname, char *strvar)
{
  return;
}

void	APISetDynamicScenery(char trueOrFalse)
{
  return;
}

void	APISetNumDynamicScenery(int numDynamicObjects)
{
  return;
}

void	APISetShadows(int shadowsValue)	// 0=off, 1=aircraft, 2=all
{
  return;
}

void	APISetFiniteFuel(char trueOrFalse)
{
  return;
}

void	APISetFiniteBattery(char trueOrFalse)
{
  return;
}

void	APISetPFactor(char trueOrFalse)
{
  return;
}

void	APISetPropTorque(char trueOrFalse)
{
  return;
}

void	APISetAutoMixture(char trueOrFalse)
{
  return;
}

void	APISetAutoPropeller(char trueOrFalse)
{
  return;
}

void	APISetIcePropeller(char trueOrFalse)
{
  return;
}

void	APISetIceEngine(char trueOrFalse)
{
  return;
}

void	APISetIceWing(char trueOrFalse)
{
  return;
}

void	APISetIcePitot(char trueOrFalse)
{
  return;
}

void	APISetGyroDrift(char trueOrFalse)
{
  return;
}

void	APISetCoupleRudderAilerons(char trueOrFalse)
{
  return;
}

void	APISetCoupleAirBrake(char trueOrFalse)
{
  return;
}

void	APISetCockpitMouseScrolling(char trueOrFalse)
{
  return;
}

void	APISetATCServiceNames(char trueOrFalse)
{
  return;
}

void	APISetATCScrollingText(char trueOrFalse)
{
  return;
}

void	APISetObjectCollisions(int collisions)	// bit 1 = ground, bit 2 = static, bit 3 = dynamic
{
  return;
}


//
//	UI functions
//

void	APICreateWindow2(unsigned long windowID, char *winFile, SDLLObject *notify)
{ //--APICreateWindow2

  // Dont create any window before simulation -------------------------------
  if (globals->appState != APP_SIMULATION)  return;
  // Dont create any window if one is already active -------------------------
  CFuiWindow *old = globals->fui->GetFuiWindow (windowID);
  if (old)  return;
  // Instantiate a new CFuiWindow object and add to the window map
  MEMORY_LEAK_MARKER ("APICreateWindow2");
  CFuiWindow *window = new CFuiDLL/*CFuiWindow*/ (windowID, winFile);
  MEMORY_LEAK_MARKER ("APICreateWindow2");
  #ifdef _DEBUG_CFuiDLL	
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
    {
      char buf1[128] = {0};
      TagToString (buf1, windowID);
      fprintf(fp_debug, "APICreateWindow2 %s %p\n", buf1, window);
      fclose(fp_debug); 
    }
  #endif
  // with CFuiDLL there's no more need to add this window to the fui list
  // globals->fui->AddFuiWindow (windowID, window);
  return;
}

int	APIIsWindowOpen(unsigned long windowID)
{
  //--APIIsWindowOpen
  return globals->fui->IsWindowCreated (windowID);
}

void	APICloseWindow2(unsigned long windowID)
{
  //--APICloseWindow2
  globals->fui->DestroyFuiWindow (windowID);
  #ifdef _DEBUG_CFuiDLL	
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
    {
      char buf1[128] = {0};
      TagToString (buf1, windowID);
      fprintf(fp_debug, "APICloseWindow2 %s\n", buf1);
      fclose(fp_debug); 
    }
  #endif
  return;
}

int	APIIsComponentType(unsigned long windowID, unsigned long componentID, unsigned int componentType)
{ //-- APIIsComponentType
  CFuiWindow *win = globals->fui->GetFuiWindow (windowID);
  if (win) {
    #ifdef _DEBUG_CFuiDLL	
      FILE *fp_debug;
      if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
      {
        char buf1[128] = {0};
        TagToString (buf1, windowID);
        char buf2[128] = {0};
        TagToString (buf2, componentID);
        fprintf(fp_debug, "APIIsComponentType %s %p %s\n", 
          buf1, win, buf2);
        fclose(fp_debug); 
      }
    #endif
    CFuiComponent *comp = win->GetComponent (componentID);
    // EFuiComponentTypes
    if (comp) {
      EFuiComponentTypes type = static_cast<EFuiComponentTypes> (componentType);
      int ret = (comp->IsType (type)) ? 1 : 0;
      return ret;
    }
  }
  return 0;
}

void	APIShowWindow(unsigned long windowID)
{ //-- APIShowWindow
  CFuiWindow *win = globals->fui->GetFuiWindow (windowID);
  if (win) {
    #ifdef _DEBUG_CFuiDLL	
      FILE *fp_debug;
      if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
      {
        char buf1[128] = {0};
        TagToString (buf1, windowID);
        fprintf(fp_debug, "APIShowWindow %s %p %d\n", 
          buf1, win, win->HasProperty(FUI_IS_VISIBLE));
        fclose(fp_debug); 
      }
    #endif
    if (!win->HasProperty(FUI_IS_VISIBLE)) {
      win->ShowMe ();
      win->SetProperty (FUI_IS_VISIBLE);
    }
  }
  return;
}

void	APIHideWindow(unsigned long windowID)
{ //-- APIHideWindow
  CFuiWindow *win = globals->fui->GetFuiWindow (windowID);
  if (win) {
    #ifdef _DEBUG_CFuiDLL	
      FILE *fp_debug;
      if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
      {
        char buf1[128] = {0};
        TagToString (buf1, windowID);
        fprintf(fp_debug, "APIHideWindow %s %p %d\n", 
          buf1, win, win->HasProperty(FUI_IS_VISIBLE));
        fclose(fp_debug); 
      }
    #endif
    if (win->HasProperty(FUI_IS_VISIBLE)) {
      win->HideMe ();
      win->RazProperty (FUI_IS_VISIBLE);
    }
  }
  return;
}


void	APIGainFocus(unsigned long windowID, unsigned long componentID)
{
  return;
}

void	APILoseFocus(unsigned long windowID, unsigned long componentID)
{
  return;
}


char*	APIGetUIButtonLabel(unsigned long windowID, unsigned long buttonID)
{ //-- APIGetUIButtonLabel
  CFuiWindow *win = globals->fui->GetFuiWindow (windowID);
  if (win) {
    #ifdef _DEBUG_CFuiDLL	
      FILE *fp_debug;
      if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
      {
        char buf1[128] = {0}; 
        char buf2[128] = {0};
        TagToString (buf1, windowID);
        TagToString (buf2, buttonID);
        fprintf(fp_debug, "APIGetUIButtonLabel %s %s %p\n", 
          buf1, buf2, win);
        fclose(fp_debug); 
      }
    #endif
    CFuiComponent *comp = win->GetComponent (buttonID);
    if (comp)
      return (comp->GetText ());
  }
  return NULL;
}

void	APISetUIButtonLabel(unsigned long windowID, unsigned long buttonID, char *label)
{ //-- APISetUIButtonLabel
  CFuiWindow *win = globals->fui->GetFuiWindow (windowID);
  if (win) {
    #ifdef _DEBUG_CFuiDLL	
      FILE *fp_debug;
      if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
      {
        char buf1[128] = {0}; 
        char buf2[128] = {0};
        TagToString (buf1, windowID);
        TagToString (buf2, buttonID);
        fprintf(fp_debug, "APISetUIButtonLabel %s %s %p %s\n", 
          buf1, buf2, win, label);
        fclose(fp_debug); 
      }
    #endif
    CFuiComponent *comp = win->GetComponent (buttonID);
    if (comp)
      comp->SetText (label);
  }
  return;
}


int	APIGetUICheckboxState(unsigned long windowID, unsigned long checkID)
{
  return 0;
}

void	APISetUICheckboxState(unsigned long windowID, unsigned long checkID, int state)
{
  return;
}

char*	APIGetUICheckboxLabel(unsigned long windowID, unsigned long checkID)
{
  return NULL;
}

void	APISetUICheckboxLabel(unsigned long windowID, unsigned long checkID, char *label)
{
  return;
}


int	APIGetUIRadioState(unsigned long windowID, unsigned long radioID)
{
  return 0;
}

void	APISetUIRadioState(unsigned long windowID, unsigned long radioID, int state)
{
  return;
}

char*	APIGetUIRadioLabel(unsigned long windowID, unsigned long radioID)
{
  return NULL;
}

void	APISetUIRadioLabel(unsigned long windowID, unsigned long radioID, char *label)
{
  return;
}

void	APIAddUIMenu(unsigned long windowID, unsigned long menubarID, unsigned long menuID, const char *label)
{
  //--APIAddUIMenu
  // main menu bar
  if (!windowID && menubarID == 'MBar') {
    globals->sdk_menu.AddMenu (menuID, label);
  }
  // new menu bar ???
  /// \todo how it works ?
  else if (windowID) {
//    CFuiManager::Instance()->AddMenu (windowID, menubarID, menuID, label);
  }

  #ifdef _DEBUG_ui	
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_ui.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "APIAddUIMenu %d %d %d %s\n", windowID, menubarID, menuID, label);
		  fclose(fp_debug); 
	  }
  #endif
}

void	APIAddUIMenuItem(unsigned long windowID, unsigned long menubarID, unsigned long menuID, unsigned long itemID, const char *label)
{
  //--APIAddUIMenuItem
  // main menu bar
  if (!windowID && menubarID == 'MBar') {
    globals->sdk_menu.AddMenuItem (menuID, itemID, label);
  }
  /// \todo how it works ?
  else if (windowID) {
 //   CFuiManager::Instance()->AddMenuItem (windowID, menubarID, menuID, itemID, label);
  }

  #ifdef _DEBUG_ui	
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_ui.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "APIAddUIMenuItem %d %d %d %d %s\n", windowID, menubarID, menuID, itemID, label);
		  fclose(fp_debug); 
	  }
  #endif
}

void	APIAddUIMenuSeperator(unsigned long windowID, unsigned long menubarID, unsigned long menuID)
{
  return;
}

void	APIRemoveUIMenuItem(unsigned long windowID, unsigned long menubarID, unsigned long menuID, unsigned long itemID)
{
  return;
}

void	APIClearUIMenuItems(unsigned long windowID, unsigned long menubarID, unsigned long menuID)
{
  return;
}

void	APICheckUIMenuItem(unsigned long windowID, unsigned long menubarID, unsigned long menuID, unsigned long itemID, int checked)
{
  return;
}

void	APIEnableUIMenuItem(unsigned long windowID, unsigned long menubarID, unsigned long menuID, unsigned long itemID, int enabled)
{
  return;
}


void	APIAddUIPopupItem(unsigned long windowID, unsigned long popupID, unsigned long itemID, const char *label)
{
  return;
}

void	APIAddUIPopupSeperator(unsigned long windowID, unsigned long popupID)
{
  return;
}

void	APIRemoveUIPopupItem(unsigned long windowID, unsigned long popupID, unsigned long itemID)
{
  return;
}

void	APIClearUIPopupItems(unsigned long windowID, unsigned long popupID)
{
  return;
}

void	APISelectUIPopupItem(unsigned long windowID, unsigned long popupID, unsigned long itemID)
{
  return;
}

unsigned long	APIGetUIPopupItemSelection(unsigned long windowID, unsigned long popupID, char *outText)
{
  return 0;
}

char*	APIGetUIPopupLabel(unsigned long windowID, unsigned long popupID)
{
  return NULL;
}

void	APISetUIPopupLabel(unsigned long windowID, unsigned long popupID, char *label)
{
  return;
}


void	APISetUIRadioGroupSelection(unsigned long windowID, unsigned long groupID, unsigned long radioID)
{
  return;
}

unsigned long	APIGetUIRadioGroupSelection(unsigned long windowID, unsigned long groupID)
{
  return 0;
}


char*	APIGetUILabelText(unsigned long windowID, unsigned long labelID)
{
  return NULL;
}

void	APISetUILabelText(unsigned long windowID, unsigned long labelID, char *label)
{
  return;
}


void	APISetUIScrollbarMin(unsigned long windowID, unsigned long scrollID, float min)
{
  return;
}

void	APISetUIScrollbarMax(unsigned long windowID, unsigned long scrollID, float max)
{
  return;
}

void	APISetUIScrollbarValue(unsigned long windowID, unsigned long scrollID, float value)
{
  return;
}

float	APIGetUIScrollbarMin(unsigned long windowID, unsigned long scrollID)
{
  return 0.0f;
}

float	APIGetUIScrollbarMax(unsigned long windowID, unsigned long scrollID)
{
  return 0.0f;
}

float	APIGetUIScrollbarValue(unsigned long windowID, unsigned long scrollID)
{
  return 0.0f;
}

void	APISetUIScrollbarPageSize(unsigned long windowID, unsigned long scrollID, float pageSize)
{
  return;
}

float	APIGetUIScrollbarPageSize(unsigned long windowID, unsigned long scrollID)
{
  return 0.0f;
}


void	APISetUISliderMajorTickSpacing(unsigned long windowID, unsigned long sliderID, float spacing)
{
  return;
}

void	APISetUISliderMinorTickSpacing(unsigned long windowID, unsigned long sliderID, float spacing)
{
  return;
}

void	APISetUISliderSnapToTicks(unsigned long windowID, unsigned long sliderID, int snapFlag)
{
  return;
}

void	APISetUISliderDrawMajorTicks(unsigned long windowID, unsigned long sliderID, int drawFlag)
{
  return;
}

void	APISetUISliderDrawMinorTicks(unsigned long windowID, unsigned long sliderID, int drawFlag)
{
  return;
}

float	APIGetUISliderMajorTickSpacing(unsigned long windowID, unsigned long sliderID)
{
  return 0.0f;
}

float	APIGetUISliderMinorTickSpacing(unsigned long windowID, unsigned long sliderID)
{
  return 0.f;
}

int	APIGetUISliderSnapToTicks(unsigned long windowID, unsigned long sliderID)
{
  return 0;
}

int	APIGetUISliderDrawMajorTicks(unsigned long windowID, unsigned long sliderID)
{
  return 0;
}

int	APIGetUISliderDrawMinorTicks(unsigned long windowID, unsigned long sliderID)
{
  return 0;
}

void	APISetUISliderLabel(unsigned long windowID, unsigned long sliderID, char *label)
{
  return;
}


void	APISetUIGroupBoxLabel(unsigned long windowID, unsigned long groupID, char *label)
{
  return;
}


void	APISetUITextMaxChars(unsigned long windowID, unsigned long textID, int maxChars)
{
  return;
}

int	APIGetUITextMaxChars(unsigned long windowID, unsigned long textID)
{
  return 0;
}

void	APIUseUITextPassword(unsigned long windowID, unsigned long textID, int passwordFlag)
{
  return;
}

int	APIIsUITextPassword(unsigned long windowID, unsigned long textID)
{
  return 0;
}

int	APIGetUITextLength(unsigned long windowID, unsigned long textID)
{
  return 0;
}

void	APISetUITextText(unsigned long windowID, unsigned long textID, char *text)
{ //-- APISetUITextText
  CFuiWindow *win = globals->fui->GetFuiWindow (windowID);
  if (win) {
    #ifdef _DEBUG_CFuiDLL	
      FILE *fp_debug;
      if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
      {
        char buf1[128] = {0};
        TagToString (buf1, windowID);
        char buf2[128] = {0};
        TagToString (buf2, textID);
        fprintf(fp_debug, "APISetUITextText %s %p %s %s\n", 
          buf1, win, buf2, text);
        fclose(fp_debug); 
      }
    #endif
    CFuiComponent *comp = win->GetComponent (textID);
    if (comp) comp->SetText (text);
  }
  return;
}

char*	APIGetUITextText(unsigned long windowID, unsigned long textID)
{ //-- APIGetUITextText
  CFuiWindow *win = globals->fui->GetFuiWindow (windowID);
  if (win) {
    #ifdef _DEBUG_CFuiDLL	
      FILE *fp_debug;
      if(!(fp_debug = fopen("__DDEBUG_CFuiDLL.txt", "a")) == NULL)
      {
        char buf1[128] = {0};
        TagToString (buf1, windowID);
        char buf2[128] = {0};
        TagToString (buf2, textID);
        fprintf(fp_debug, "APIGetUITextText %s %p %s\n", 
          buf1, win, buf2);
        fclose(fp_debug); 
      }
    #endif
    CFuiComponent *comp = win->GetComponent (textID);
    if (comp) return (comp->GetText ());
  }
  return NULL;
}

void	APISetUITextSelection(unsigned long windowID, unsigned long textID, int firstChar, int lastChar)
{
  return;
}

void	APIGetUITextSelection(unsigned long windowID, unsigned long textID, int *firstChar, int *lastChar)
{
  return;
}


void	APISetUIProgressIndeterminate(unsigned long windowID, unsigned long progressID, int flag)
{
  return;
}

int	APIGetUIProgressIndeterminate(unsigned long windowID, unsigned long progressID)
{
  return 0;
}

void	APISetUIProgressMin(unsigned long windowID, unsigned long progressID, int min)
{
  return;
}

int	APIGetUIProgressMin(unsigned long windowID, unsigned long progressID)
{
  return 0;
}

void	APISetUIProgressMax(unsigned long windowID, unsigned long progressID, int max)
{
  return;
}

int	APIGetUIProgressMax(unsigned long windowID, unsigned long progressID)
{
  return 0;
}

void	APISetUIProgressValue(unsigned long windowID, unsigned long progressID, int value)
{
  return;
}

int	APIGetUIProgressValue(unsigned long windowID, unsigned long progressID)
{
  return 0;
}


void	APIScrollUIListToRow(unsigned long windowID, unsigned long listID, int row)
{
  return;
}

void	APIScrollUIListToColumn(unsigned long windowID, unsigned long listID, int column)
{
  return;
}

int	APIGetUIListRowCount(unsigned long windowID, unsigned long listID)
{
  return 0;
}

int	APIGetUIListColumnCount(unsigned long windowID, unsigned long listID)
{
  return 0;
}

void	APIAddUIListColumn(unsigned long windowID, unsigned long listID, int width, char *title)	// neg. width is "minimum" width)
{
  return;
}

void	APIDeleteUIListColumn(unsigned long windowID, unsigned long listID, int column)
{
  return;
}

void	APIDeleteAllUIListColumns(unsigned long windowID, unsigned long listID)
{
  return;
}

void	APISetUIListColumnWidth(unsigned long windowID, unsigned long listID, int column, int width)
{
  return;
}

void	APIAddUIListRow(unsigned long windowID, unsigned long listID, int height)
{
  return;
}

void	APIDeleteUIListRow(unsigned long windowID, unsigned long listID, int row)
{
  return;
}

void	APIDeleteAllUIListRows(unsigned long windowID, unsigned long listID)
{
  return;
}

int	APICountUIListSelectedRows(unsigned long windowID, unsigned long listID)
{
  return 0;
}

int	APIGetUIListSelectedRow(unsigned long windowID, unsigned long listID, int index)
{
  return 0;
}

void	APISelectUIListRow(unsigned long windowID, unsigned long listID, int row)
{
  return;
}

void	APIClearUIListSelection(unsigned long windowID, unsigned long listID)
{
  return;
}

void	APISetUIListCellText(unsigned long windowID, unsigned long listID, int row, int column, char *text)
{
  return;
}

void	APISetUIListCellIcon(unsigned long windowID, unsigned long listID, int row, int column, char *filename)
{
  return;
}

void	APIClearUIListCellText(unsigned long windowID, unsigned long listID, int row, int column)
{
  return;
}

void	APIClearUIListCellIcon(unsigned long windowID, unsigned long listID, int row, int column)
{
  return;
}

void	APIGetUIListCellText(unsigned long windowID, unsigned long listID, int row, int column, char *outtext)
{
  return;
}


void	APIAddUIGraphTrace(unsigned long windowID, unsigned long graphID, unsigned long traceID, int traceType)
{
  return;
}

void	APIRemoveUIGraphTrace(unsigned long windowID, unsigned long graphID, unsigned long traceID)
{
  return;
}

void	APIRemoveAllUIGraphTraces(unsigned long windowID, unsigned long graphID)
{
  return;
}

void	APIAddUIGraphTracePoint(unsigned long windowID, unsigned long graphID, unsigned long traceID, float x, float y)
{
  return;
}

void	APIClearUIGraphTracePoints(unsigned long windowID, unsigned long graphID, unsigned long traceID)
{
  return;
}

void	APISetUIGraphTraceRange(unsigned long windowID, unsigned long graphID, unsigned long traceID, float minX, float minY, float maxX, float maxY)
{
  return;
}

void	APISetUIGraphTraceColor(unsigned long windowID, unsigned long graphID, unsigned long traceID, unsigned int color)
{
  return;
}

void	APISetUIGraphUseGrid(unsigned long windowID, unsigned long graphID, int useGrid)
{
  return;
}

void	APISetUIGraphGridRange(unsigned long windowID, unsigned long graphID, float minX, float minY, float maxX, float maxY)
{
  return;
}

void	APISetUIGraphGridSpacing(unsigned long windowID, unsigned long graphID, float xStep, float yStep)
{
  return;
}

void	APISetUIGridColor(unsigned long windowID, unsigned long graphID, unsigned int color)
{
  return;
}

void	APISetUIGridBackColor(unsigned long windowID, unsigned long graphID, unsigned int color)
{
  return;
}

void	APIGetUIGraphGridRange(unsigned long windowID, unsigned long graphID, float *minX, float *minY, float *maxX, float *maxY)
{
  return;
}

void	APIGetUIGraphGridSpacing(unsigned long windowID, unsigned long graphID, float *xStep, float *yStep)
{
  return;
}


void	APISetUIPictureImage(unsigned long windowID, unsigned long pictureID, char *pbmFile)
{
  return;
}


//
//	standard dialog boxes
//

void	APIOpenFileDialog(unsigned long windowID, char *title, char *extension, char *defaultName, char *defaultFolder, int createFolder, int allowLocalFiles, int allowPodFiles, int fileMustExist, SDLLObject *notify)
{
  return;
}

void	APISaveFileDialog(unsigned long windowID, char *title, char *extension, char *defaultName, char *defaultFolder, int createFolder, int allowLocalFiles, int allowPodFiles, int fileMustExist, SDLLObject *notify)
{
  return;
}

void	APIGetFilePath(unsigned long windowID, char *pathBuffer, int bufferLen)
{
  return;
}

void	APIAlertBox(unsigned long windowID, char *title, char *message, int buttonStyle, int iconStyle, SDLLObject *notify)
{
  return;
}

void	APIOKCancelBox(unsigned long windowID, char *message, char *okButton, char *cancelButton, SDLLObject *notify)
{
  return;
}

void	APIDebugString(char *debug)
{
  return;
}


//
//	Python specific functions (internal use only)
//

int	APICountPythonMethodModules(void)
{
  return 0;
}

void	APIGetPythonMethodModule(int index, char *moduleName, SPythonMethod **methods)
{
  return;
}


//
//	EPIC hardware support
//

int	APIEpicAvailable(void)
{
  return 0;
}

void	APIEpicRead(unsigned char *buffer, int address, int numBytes)
{
  return;
}

void	APIEpicWrite(unsigned char *buffer, int address, int numBytes)
{
  return;
}

int	APIEpicSendEvent(SEpicEvent *event)
{
  return 0;
}

int	APIEpicSendPH(int phNum, int data0, int data1, int data2, int data3)
{
  return 0;
}

int	APIEpicSendQP(int qpNum)
{
  return 0;
}


//
//	Data Block support
//

int	APIOpenDataBlock(const char* blockName, const char* filename) // adds to list
{
  return 0;
}

void	APICloseDataBlock(const char* blockName) // removes from list
{
  return;
}

int	APIDataBlockSize(const char* blockName)
{
  return 0;
}

int	APIReadDataBlock(const char* blockName, SDataBlockItem *array)
{
  return 0;
}

int	APIWriteDataBlock(const char* blockName, SDataBlockItem *array)
{
  return 0;
}

int	APIReadDataItem(const char *blockName, unsigned int label, SDataBlockItem *item)
{
  return 0;
}

int	APIReadDataItemInt(const char *blockName, unsigned int label)
{
  return 0;
}

float	APIReadDataItemFloat(const char *blockName, unsigned int label)
{
  return 0.0f;
}

int	APIWriteDataItem(const char *blockName, SDataBlockItem *item)
{
  return 0;
}

int	APIWriteDataItemInt(const char *blockName, unsigned int label, int data)
{
  return 0;
}

int	APIWriteDataItemFloat(const char *blockName, unsigned int label, float data)
{
  return 0;
}



