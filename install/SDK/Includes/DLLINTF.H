/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
//	DLLINTF.H
//
//
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

#ifndef __DLLINTF__
#define __DLLINTF__

#ifndef __MAINSDK__
#include "sdk.h"
#endif

extern "C" {

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
//	Function Prototypes
//
//	The functions listed below are optional functions
//	the .DLL can implement to allow  specific
//	extensions within main application.  The .DLL will be passed
//	a pointer to a structure containing pointers
//	to functions that match the functions below; you
//	should populate this structure with pointers to
//	the functions you have implemented, and leave
//	the others alone.  The Sim will make appropriate
//	callbacks into the .DLL as needed based on the
//	functions implemented.
//
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLTest(void)
//
//
//	Params:
//		None.
//
//	Usage:
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLTestIntf)(void);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLStartSituation(void)
//
//
//	Params:
//		None.
//
//	Usage:
//		Legacy calls this function whenever a new
//		flight situation is started, either through
//		the Legacy Now screen or when a flight plan
//		is restarted.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLStartSituationIntf)(void);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLEndSituation(void)
//
//
//	Params:
//		None.
//
//	Usage:
//		Legacy calls this function whenever a
//		flight situation is stopped by the
//		user.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLEndSituationIntf)(void);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLDestroyObject(SDLLObject *object)
//
//
//	Params:
//		A pointer to the object being destroyed.
//
//	Usage:
//		Legacy calls this function whenever an object
//		under Fly's control is being destroyed,
//		such as when an object goes out of scope
//		or the user explicitly destroys the object
//		(closing a window or cockpit).  If you
//		allocated storage for the object, you 
//		should delete it when you are called.
//		This also means that you must implement
//		this function if you are implementing
//		new objects that will be instantiated
//		through the .DLL (new gauges, systems, etc.)
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLDestroyObjectIntf)(SDLLObject*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLReceiveMessage(
//			SDLLObject *object,
//			SMessage *message)
//
//	Params:
//		object	-	the object the message
//				is directed to
//
//		message	-	the contents of the message
//				to be processed
//
//	Usage:
//		This function allows a .DLL to "share"
//		data it generates with other systems and
//		objects in the Legacy environment. Messages
//		are the primary method of attaching 
//		simulated data with visual gauges.
//
//		When a message is received, the .DLL should
//		check the message parameters to see if it
//		is the intended recipient of the message;
//		if it is, the appropriate data member
//		should be populated with return data and
//		the result code MSG_PROCESSED should be
//		returned.  Otherwise, MSG_IGNORED should
//		be returned to allow other objects the
//		chance to intercept and process the 
//		message.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLReceiveMessageIntf)(SDLLObject*,SMessage*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLInstantiate(
//			const long baseID,
//			const long classID,
//			SDLLObject **object)
//
//
//	Params:
//		baseID	-	the base class type being
//				requested; .DLL's should 
//				only attempt to process
//				those matching EDLLObjectType
//				types
//
//		classID	-	unique class id being
//				requested; this should be
//				unique both to the .DLL and
//				to the Legacy system as well
//
//		object	-	.DLL should return a non-NULL
//				*object result if the .DLL
//				instantiated an object for
//				this request.
//
//	Usage:
//		Legacy calls this function whenever a new
//		object needs to be created, such as when
//		a cockpit panel file is processed and the
//		gauge type does not match one of Legacy known
//		types.  In this case, Legacy will call each
//		.DLL that registered the correct baseID and
//		classID types that match the requested
//		object, and the first .DLL that returns
//		a non-NULL object reference is then "bound"
//		to an equivalent proxy object within Legacy
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLInstantiateIntf)(const long,const long,SDLLObject**);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLInitObject(SDLLObject *object)
//
//
//	Params:
//		object	-	The object to initialize
//
//	Usage:
//		Legacy calls this function after an object
//		is created, usually at a "sync" point;
//		for example, for gauges this function
//		is called once the entire panel file has
//		be read and all gauges have been created;
//		this allows gauges to reference other
//		gauges, and guarantees that when the 
//		DLLInitObject routine is called for an
//		object the Legacy simulation is in a known
//		state.  You should use this opportunity
//		to register key handlers, initialize
//		variables, or establish message connections
//		with other objects.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLInitObjectIntf)(SDLLObject*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLIdle(float deltaTime)
//
//
//	Params:
//		deltaTime -	The number of seconds
//				(fractional) elapsed since
//				the last call to DLLIdle
//
//	Usage:
//		Legacy calls this function constantly for
//		each DLL that implements this routine.
//		This can be used to manipulate Legacy without
//		requiring the instantiation of a specific
//		object.  Be warned, however, that since
//		this routine is called all the time, any
//		slow performance in a single .DLL can 
//		significantly degrade the entire simulation.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLIdleIntf)(float);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLDraw(
//			SDLLObject *object,
//			SSurface *surface)
//
//
//	Params:
//		object -	The object that needs to
//				draw to the screen
//
//		surface -	The surface to draw output
//				into
//
//	Usage:
//		Legacy will automatically call this function
//		when a gauge or window object needs to
//		draw it's output to the screen.  A drawing
//		surface will be provided; all drawing must
//		occur within the boundaries of this surface,
//		which will be determined either by the
//		gauge <size> entry, or the current window
//		size.  Whatever is drawn into the surface
//		will be blitted to the final output of Legacy
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLDrawIntf)(SDLLObject*,SSurface*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLRead(
//			SDLLObject *object,
//			SStream *stream,
//			unsigned int tag)
//
//
//	Params:
//		object -	The object whose data is
//				being read from the stream
//
//		stream -	A pointer to the stream
//				data will be read from
//
//		tag -		The unique data tag for
//				the next piece of data
//				to be read
//
//	Usage:
//		Legacy stores data in files having a tagged
//		format, represented by <xxxx>.  A tag can
//		be introduced by an object, as long as
//		it is unique within that object.  A tag
//		can contain any number of data elements.
//		When a tag is processed by Legacy, it notifies
//		the current object to process the tag.
//		The object should respond by using the
//		appropriate APIRead... calls to read each
//		data element in the order they were
//		written to disk.
//
//		The function should return TAG_READ if
//		the tag was processed, or TAG_IGNORED if
//		it was not.
//
//		If you choose to implement this function
//		for custom data tags, you should also
//		implement the corresponding DLLWrite
//		routine to save tags back to the stream.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLReadIntf)(SDLLObject*,SStream*,unsigned int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLReadFinished(SDLLObject *object)
//
//
//	Params:
//		object	-	A pointer to the object
//				that was just read from
//				a stream
//
//	Usage:
//		Legacy will call this routine once all data
//		tags for an object have been processed
//		from the stream.  This allows an object
//		to initialize any members that were 
//		dependent on all data tags being read.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLReadFinishedIntf)(SDLLObject*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLWrite(
//			SDLLObject *object,
//			SStream *stream)
//
//
//	Params:
//		object	-	A pointer to the object
//				to write to the stream
//
//		stream -	A pointer to the stream to
//				write data to
//
//	Usage:
//		Legacy calls this function whenever an 
//		object needs to save it's current state
//		to a stream.  You should save any data
//		necessary to place the object back into
//		it's current state at a later time 
//		(if loaded from a saved game stream, for
//		example.)
//
//		You can write tags in any order you choose,
//		since the DLLRead routine is not dependent
//		on a particular order.  If you implement
//		DLLWrite, you should also implement the
//		DLLRead routine.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLWriteIntf)(SDLLObject*,SStream*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		EClickResult DLLMouseClick(
//			SDLLObject *object,
//			SSurface *surface,
//			int mouseX,
//			int mouseY,
//			int mouseButtons)
//
//
//	Params:
//		object -	The object being notified
//				of the mouse click processing
//				(normally a gauge or window
//				object)
//
//		surface -	The drawing surface for the
//				object (used to query 
//				dimensions for the object)
//
//		mouseX -	The mouse X coordinate, in
//				local coordinates of the
//				surface
//
//		mouseY -	The mouse Y coordinate, in
//				local coordindates of the
//				surface
//
//		mouseButtons -	A bitfield of the mouse
//				button states; AND against
//				EMouseButton values to
//				determine which buttons
//				are down
//
//	Usage:
//		When a gauge or window object receives
//		a mouse down event, Legacy will call this
//		routine on the initial click to allow
//		the object to determine what part was
//		hit and what the result should be.  If
//		the item clicked does not need mouse 
//		tracking, return MOUSE_TRACKING_OFF.  If
//		the item needs tracking, return
//		MOUSE_TRACKING_ON.  In this case, Legacy
//		will then continuously call the DLLTrackClick
//		routine each frame while the mouse button
//		remains down, then DLLStopClick when the
//		mouse button is released.  You must 
//		implement these routines to support 
//		mouse tracking.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef EClickResult(DLLMouseClickIntf)(SDLLObject*,SSurface*,int,int,int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		EClickResult DLLTrackClick(
//			SDLLObject *object,
//			SSurface *surface,
//			int mouseX,
//			int mouseY,
//			int mouseButtons)
//
//
//	Params:
//		object -	The object being notified
//				of the mouse click processing
//				(normally a gauge or window
//				object)
//
//		surface -	The drawing surface for the
//				object (used to query 
//				dimensions for the object)
//
//		mouseX -	The mouse X coordinate, in
//				local coordinates of the
//				surface
//
//		mouseY -	The mouse Y coordinate, in
//				local coordindates of the
//				surface
//
//		mouseButtons -	A bitfield of the mouse
//				button states; AND against
//				EMouseButton values to
//				determine which buttons
//				are down
//
//	Usage:
//		If a gauge receives a DLLMouseClick 
//		callback and requests mouse tracking by
//		returning MOUSE_TRACKING_ON, Legacy will
//		continue to call the object with updated
//		mouse coordinates each frame until either
//		the mouse button is released, or if
//		DLLTrackClick returns MOUSE_TRACKING_OFF.
//		MOUSE_TRACKING_ON should be returned to
//		continue tracking.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef EClickResult(DLLTrackClickIntf)(SDLLObject*,SSurface*,int,int,int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		EClickResult DLLStopClick(
//			SDLLObject *object,
//			SSurface *surface,
//			int mouseX,
//			int mouseY,
//			int mouseButtons)
//
//
//	Params:
//		object -	The object being notified
//				of the mouse click processing
//				(normally a gauge or window
//				object)
//
//		surface -	The drawing surface for the
//				object (used to query 
//				dimensions for the object)
//
//		mouseX -	The mouse X coordinate, in
//				local coordinates of the
//				surface
//
//		mouseY -	The mouse Y coordinate, in
//				local coordindates of the
//				surface
//
//		mouseButtons -	A bitfield of the mouse
//				button states; AND against
//				EMouseButton values to
//				determine which buttons
//				are down
//
//	Usage:
//		This function is called when mouse tracking
//		has ceased for an object, either because
//		the mouse button was released or the 
//		DLL returned MOUSE_TRACKING_OFF from the
//		DLLTrackClick routine.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLStopClickIntf)(SDLLObject*,SSurface*,int,int,int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		ECursorResult DLLMouseMoved(
//			SDLLObject *object,
//			SSurface *surface,
//			int mouseX,
//			int mouseY)
//
//
//	Params:
//		object -	The object being notified
//				of the mouse click processing
//				(normally a gauge or window
//				object)
//
//		surface -	The drawing surface for the
//				object (used to query 
//				dimensions for the object)
//
//		mouseX -	The mouse X coordinate, in
//				local coordinates of the
//				surface
//
//		mouseY -	The mouse Y coordinate, in
//				local coordindates of the
//				surface
//
//	Usage:
//		This routine is called whenever the mouse
//		is over a window or gauge to allow the
//		mouse cursor to be changed or highlighting
//		to occur in the gauge.  If the cursor
//		is changed with the APISetCursor call, you
//		should return CURSOR_WAS_CHANGED, otherwise
//		return CURSOR_NOT_CHANGED.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef ECursorResult(DLLMouseMovedIntf)(SDLLObject*,SSurface*,int,int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLPrepare(SDLLObject *object)
//
//	Params:
//		object -	The object being prepared
//
//	Usage:
//		Legacy calls this routine just before the
//		simulation is started; it is the last 
//		chance for an object to prepare itself
//		before simulating.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLPrepareIntf)(SDLLObject*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLTimeSlice(
//			SDLLObject *object,
//			const float deltaTime)
//
//	Params:
//		object -	The object being given
//				a time slice
//
//		deltaTime -	The number of seconds
//				(fractional) since the
//				last call to DLLTimeSlice
//
//	Usage:
//		DLLTimeSlice is called to allow a system
//		or object to have an opportunity to run
//		a custom simulation loop.  The deltaTime
//		parameter will provide the number of 
//		seconds that have passed since the previous
//		call.  Since DLLTimeSlice is called during
//		the critical loop of the Legacy simulation
//		system, beware that large amounts of CPU
//		time here will have a negative impact on
//		the overall frame rate for Legacy as a whole.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLTimeSliceIntf)(SDLLObject*,const float);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLKeyPressed(
//			int keyCode,
//			int modifiers)
//
//	Params:
//		keyCode -	Values as defined in the
//				EKeyboardKeys enumeration
//
//		modifiers -	OR'ed collection of modifier
//				keys as defined in
//				EKeyboardModifiers
//
//	Usage:
//		When implemented, this routine will be
//		called whenever a key press occurs in Legacy.
//		You can use this to trigger the creation
//		of other objects or events, though you 
//		should try to avoid capturing standard
//		Legacy key assignments.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLKeyPressedIntf)(int,int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLKeyIntercept(
//			int keyCode,
//			int modifiers)
//
//	Params:
//		keyCode -	Values as defined in the
//				EKeyboardKeys enumeration
//
//		modifiers -	OR'ed collection of modifier
//				keys as defined in
//				EKeyboardModifiers
//
//	Returns:
//		Return a non-zero value if the keystroke
//		should be "captured" and not reported
//		to Legacy ("eating" the event), or return
//		zero if Legacy should process the keystroke.
//
//	Usage:
//		When implemented, this routine will be
//		called whenever a key press occurs in Legacy.
//		You can use this to trigger the creation
//		of other objects or events, though you 
//		should try to avoid capturing standard
//		Legacy key assignments.
//
//		The routine differs from DLLKeyPressed
//		by allowing you to silence the keystroke
//		before it is reported to Legacy
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLKeyInterceptIntf)(int,int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLKeyCallback(
//			int keyCode,
//			int modifiers,
//			SDLLObject *object)
//
//	Params:
//		keyCode -	Values as defined in the
//				EKeyboardKeys enumeration
//
//		modifiers -	OR'ed collection of modifier
//				keys as defined in
//				EKeyboardModifiers
//
//		object -	The object that registered
//				the key handler
//
//	Usage:
//		Instead of having to use DLLKeyPressed
//		to monitor individual keystrokes, you
//		can register a callback routine to be
//		called when a specific key combination
//		is pressed.  This callback is set by
//		using the APIAddKeyHandler routine.
//
//		Whenever the registered keycode and modifier
//		is pressed, the DLLKeyCallback routine will
//		be called with the keycode/modifier combo,
//		and a pointer to the object that registered
//		the handler.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLKeyCallbackIntf)(int,int,SDLLObject*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLKeyCallback2(
//			int keyID,
//			int keyCode,
//			int modifiers,
//			SDLLObject *object)
//
//	Params:
//		keyID -		The unique ID for the
//				key pressed
//
//		keyCode -	Values as defined in the
//				EKeyboardKeys enumeration
//
//		modifiers -	OR'ed collection of modifier
//				keys as defined in
//				EKeyboardModifiers
//
//		object -	The object that registered
//				the key handler
//
//	Usage:
//		Instead of having to use DLLKeyPressed
//		to monitor individual keystrokes, you
//		can register a callback routine to be
//		called when a specific key combination
//		is pressed.  This callback is set by
//		using the APIAddKeyHandler routine.
//
//		Whenever the registered keycode and modifier
//		is pressed, the DLLKeyCallback routine will
//		be called with the keycode/modifier combo,
//		and a pointer to the object that registered
//		the handler.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLKeyCallback2Intf)(int,int,int,SDLLObject*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLProcessSoundEffects(
//			SDLLObject *object)
//
//	Params:
//		object -	The object that needs to
//				process sound effects
//
//	Usage:
//		When implemented, Legacy will call this
//		routine for objects of type 
//		TYPE_DLL_WORLDOBJECT to allow updating
//		of sound effects for the object.  
//
//		Typically you will use this routine to
//		determine when a sound should be started,
//		or to update the position, velocity,
//		volume, or frequency of a sound that
//		was previously started.  If a sound
//		effect is a "one shot" effect, there is
//		no reason to further manage the sound.
//		However, if the sound is an infinite
//		looping sound, such as an engine sound,
//		this routine is where you would adjust
//		the parameters of the sound each frame.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLProcessSoundEffectsIntf)(SDLLObject*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLSuspendSoundEffects(
//			SDLLObject *object)
//
//	Params:
//		object -	The object that needs to
//				suspend sound effects
//
//	Usage:
//		This routine is called when Legacy is either
//		leaving the simulation loop, or the user
//		has paused the simulation (with Pause or
//		Slew).  You should stop all sound effects
//		that are in progress.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLSuspendSoundEffectsIntf)(SDLLObject*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLResumeSoundEffects(
//			SDLLObject *object)
//
//	Params:
//		object -	The object that needs to
//				process sound effects
//
//	Usage:
//		If Legacy suspended sound effects, it will
//		call this routine when sound effects are
//		resumed.  You can either use this routine
//		to re-trigger sounds in progress, or simply
//		wait until the next DLLProcessSoundEffects
//		call to re-trigger.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLResumeSoundEffectsIntf)(SDLLObject*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLInWindowRegion(
//			SDLLObject *object,
//			SSurface *surface,
//			int mouseX,
//			int mouseY)
//
//	Params:
//		object -	The window object created
//				by the DLL
//
//		surface -	The surface representing
//				the visible output area of
//				the window
//
//		mouseX -	The mouse X coordinate in
//				local coordinates
//
//		mouseY -	The mouse Y coordinate in
//				local coordinates
//
//	Usage:
//		If your DLL creates custom windows, you
//		should implement this function to notify
//		Legacy to which window parts are covered by
//		the mouse.  You should return one of the
//		values defined in the EWindowRegion enum.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLInWindowRegionIntf)(SDLLObject*,SSurface*,int,int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLGetPopupHelp(
//			SDLLObject *object,
//			char *string1,
//			char *string2,
//			char *string3)
//
//	Params:
//		object -	The window object created
//				by the DLL
//
//		string1 -	The first line of popup
//				help text
//
//		string2 -	The second line of popup
//				help text
//
//		string3 -	The third line of popup
//				help text
//
//	Usage:
//		You can implement this routine to provide
//		custom popup help for your gauge items.
//		Usually this is to allow you to display
//		current output of the gauge in text format,
//		such as engine RPM or oil temperature.
//		Up to three lines can be populated; the
//		strings passed in string1, string2, and
//		string3 are pointers to C-strings up to
//		63 characters in length (plus a terminating
//		NULL character.)
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLGetPopupHelpIntf)(SDLLObject*,char*,char*,char*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLUpdateCamera(
//			SDLLObject *object,
//			SPosition *targetPosition,
//			SVector *targetOrientation,
//			SPosition *eyePosition,
//			SVector *eyeOrientation,
//			const float deltaTime)
//
//	Params:
//		object -	The camera object created
//				by the DLL
//
//		targetPosition - The lat/lon position of
//				the object being looked
//				at by the camera
//
//		targetOrientation - The body orientation
//				of the object being looked
//				at
//
//		eyePosition - The computed position of the
//				eye
//
//		eyeOrientation - The computed orientation
//				of the eye
//
//		deltaTime - time (in secs) since last update
//
//	Usage:
//		This routine allows a DLL camera to take
//		the position of the active camera object
//		(normally the user's vehicle) and calculate
//		the location of where the 3D eye should be
//		placed, as well as it's orientation.
//		
//		You should not alter the values of
//		targetPosition or targetOrientation, but
//		only eyePosition and eyeOrientation.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLUpdateCameraIntf)(SDLLObject*,SPosition*,SVector*,SPosition*,SVector*,const float);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLIsInteriorCamera(SDLLObject *object)
//
//	Params:
//		object -	The camera object created
//				by the DLL
//
//	Usage:
//		This routine notifies Legacy if the camera
//		is considered an interior or exterior camera.
//		If the camera is an interior camera, Legacy
//		will display the active cockpit panel but
//		will not display the 3D model of the user's
//		aircraft.
//
//		This routine should return 1 if the camera
//		is an interior camera, or zero (0) if it
//		is an exterior camera.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLIsInteriorCameraIntf)(SDLLObject*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLGetCameraName(
//			SDLLObject *object,
//			char *name,
//			int maxLength)
//
//	Params:
//		object -	The camera object created
//				by the DLL
//
//		name -		A pointer to the string
//				for the name of the
//				camera
//
//		maxLength -	The maximum length of the
//				camera name; do NOT exceed
//				this length when populating
//				the name variable
//
//	Usage:
//		This routine allows you to define the name
//		of the camera as it should appear in the
//		lower-left corner when the camera is 
//		activated with the camera key.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLGetCameraNameIntf)(SDLLObject*,char*,int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLDrawOverlay(
//			SSurface *surface,
//			SDLLObject *refObject,
//			int viewOutputX1,
//			int viewOutputY1,
//			int viewOutputX2,
//			int viewOutputY2)
//
//	Params:
//		surface -	The surface to draw the
//				overlay into
//
//		refObject -	A reference object provided
//				by the DLL; this can be NULL
//				or a pointer to any DLL
//				object you want passed
//				back to the DLL as context
//				for the overlay
//
//		viewOutputX1 -	The upperleft X coordinate
//				of the primary viewport
//				in the display area
//
//		viewOutputY1 -	The upperleft Y coordinate
//				of the primary viewport
//				in the display area
//
//		viewOutputX2 -	The lowerright X coordinate
//				of the primary viewport
//				in the display area
//
//		viewOutputY2 -	The lowerright Y coordinate
//				of the primary viewport
//				in the display area
//
//	Usage:
//		You can use this routine, in conjunction
//		with the APICreateOverlay routine to
//		set aside a portion of the output display
//		for an overlay, such as text or a graphic.
//		This can be used when a window is not
//		appropriate for your needs, but you need
//		to output to the main display.
//
//		You should call APIRemoveOverlay when you
//		no longer need the overlay.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLDrawOverlayIntf)(SSurface*,SDLLObject*,int,int,int,int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLReadIniFile(void)
//
//	Params:
//		None.
//
//	Usage:
//		This routine allows you read custom
//		.INI settings from the FLY.INI file.
//		It is suggested that you provide the
//		name of your product for the section
//		name when calling APIGetIniVar.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLReadIniFileIntf)(void);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLWriteIniFile(void)
//
//	Params:
//		None.
//
//	Usage:
//		This routine allows you write custom
//		.INI settings from the FLY.INI file.
//		It is suggested that you provide the
//		name of your product for the section
//		name when calling APISetIniVar.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLWriteIniFileIntf)(void);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLControlCanBeUsed(void)
//
//	Params:
//		None.
//
//	Usage:
//		This routine returns whether the control
//		input device can currently be used.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLControlCanBeUsedIntf)(void);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLControlButtonCount(void)
//
//	Params:
//		None.
//
//	Usage:
//		This routine returns the number of buttons
//		available on the input device.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLControlButtonCountIntf)(void);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLControlButtonPressed(int buttonIndex)
//
//	Params:
//		buttonIndex -	The index of the button
//				being tested
//
//	Usage:
//		This routine checks to see if a particular
//		button is pressed.  If the button is 
//		currently pressed, this routine should
//		return 1, otherwise return 0.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLControlButtonPressedIntf)(int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLControlButtonName(
//			int buttonIndex,
//			char *outName,
//			char *outDevice)
//
//	Params:
//		buttonIndex -	The index of the button
//				
//		outName -	The end-user visible name
//				for this button
//
//		outDevice -	The end-user visible name
//				for this device
//
//	Usage:
//		This routine should populate the outName
//		and outDevice members with the name and
//		device for the button index.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLControlButtonNameIntf)(int,char*,char*);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLControlDebounceButton(int buttonIndex)
//
//	Params:
//		buttonIndex -	The button to debounce
//
//	Usage:
//		This routine should clear any button-down
//		state for the button index pressed. This is
//		used for input routines that are triggered
//		on the initial button press, then need to
//		clear the button down state until the 
//		button is physically released.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLControlDebounceButtonIntf)(int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLControlHasAxis(EJoystickAxis axis)
//
//	Params:
//		axis -	The axis to be queried.
//
//	Usage:
//		This routine should return whether the
//		requested axis is available on the input
//		device. If the axis is present, return 1
//		else return 0.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLControlHasAxisIntf)(EJoystickAxis axis);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		float DLLControlGetAxis(EJoystickAxis axis)
//
//	Params:
//		axis -	The axis to be queried.
//
//	Usage:
//		This routine should return the present
//		position of the requested axis, returning
//		a value from 0.0 to 1.0 inclusive.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef float(DLLControlGetAxisIntf)(EJoystickAxis axis);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		unsigned int DLLControlGetIdentifier(void)
//
//	Params:
//		None.
//
//	Usage:
//		This routine returns a unique 4-char 
//		identifier for the input device.
//
//		THIS ROUTINE MUST BE IMPLEMENTED!!!
//
//		Values that are reserved by Legacy and 
//		should NOT be returned are:
//			'cont'
//			'keyb'
//			'joys'
//			'vehi'
//			'PFC '
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef unsigned int(DLLControlGetIdentifierIntf)(void);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLRunScript(
//				const char *scriptFile, 
//				const char *refName, 
//				const char runOnce)
//
//	Params:
//		scriptFile -	The pathname of the script
//				to run
//
//		refName	-	A reference name to associate
//				with the script
//
//		runOnce -	Designates whether the
//				script only runs once (=1),
//				or is persistant (=0)
//
//	Usage:
//
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLRunScriptIntf)(const char *scriptFile, const char *refName, const char runOnce);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLCallScriptFunction(
//				const char *scriptFile, 
//				const char *funcName)
//
//	Params:
//		scriptFile -	The pathname of the script
//				to run
//
//		funcName -	Function name to call (must
//				be a function taking no
//				parameters)
//
//	Usage:
//
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLCallScriptFunctionIntf)(const char *scriptFile, const char *funcName);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLStopScript(const char *scriptFile)
//
//	Params:
//		scriptFile -	The pathname of the script
//				to stop
//
//	Usage:
//
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLStopScriptIntf)(const char *scriptFile);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLStopScriptByRefName(const char *refName)
//
//	Params:
//		refName -	A reference string used
//				when a script was executed
//
//	Usage:
//
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLStopScriptByRefNameIntf)(const char *refName);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLGetScriptInfo(
//				int *numScripts,
//				SScriptInfo **array)
//
//	Params:
//		numScripts	Number of scripts recorded
//				in the array parameter
//
//		array		A sequential array of 
//				SScriptInfo entries for
//				each active script
//
//	Usage:	You MUST allocate the array using APIAllocMem,
//		even if only a single script is active.
//		If no scripts are active you do not have
//		to set the array parameter, but should
//		set numScripts = 0. Legacy will release
//		the memory storage for the array.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLGetScriptInfoIntf)(int *numScripts, SScriptInfo **array);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		SPythonMethod* DLLGetPythonMethodTableIntf(void)
//
//	Params:
//		None.
//
//	Usage:	You must allocate a memory block of
//		SPythonMethod structs and return a
//		pointer to the block. The last block
//		entry should be a NULL sentinel entry
//		to mark the end of the array.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef SPythonMethod*(DLLGetPythonMethodTableIntf)(void);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLBeginFrame(void)
//
//	Params:
//		None.
//		
//	Usage:
//		Called when each visual frame begins
//		within the simulation.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLBeginFrameIntf)(void);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLEventNotice(
//			SDLLObject *notify,
//			unsigned long id,
//			unsigned long event,
//			unsigned long subevent);
//
//	Params:
//		notify	-	The DLL object to notify
//		window	-	The window containing the component
//		id	-	The component triggering the event
//		event	-	The event
//		subevent -	The sub event
//		
//	Usage:
//		Called when a UI component triggers
//		an event.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLEventNoticeIntf)(SDLLObject*,unsigned long,unsigned long,unsigned long,unsigned long);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLEpicEvent(
//			unsigned char cmd,
//			unsigned char data[6]);
//
//	Params:
//		cmd	-	The EPIC command received
//		data	-	The associated command data
//		
//	Usage:
//		Called when an EPIC command is received
//		by Legacy
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLEpicEventIntf)(unsigned char, unsigned char[6]);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		int DLLGenerateTileScenery(
//			SPosition *ll,
//			SPosition *ur,
//			int tileType,
//			int rightType,
//			int bottomType,
//			int cornerType,
//			int gridX,
//			int gridY)
//
//	Params:
//		ll	-	Lower left lat/lon tile corner
//		ur	-	Upper right lat/lon tile corner
//		tileType	-	Ground type for this tile
//		rightType	-	Ground type for tile to the right
//		bottomType	-	Ground type for tile to the bottom
//		cornerType	-	Ground type for tile to the lower right
//		gridX		-	Plate X position of tile
//		gridY		-	Plate Y position of tile
//		
//
//	Returns:	Zero (0) if no objects created,
//			or 1 if objects were created
//
//	Usage:
//		Called when a ground tile should generate
//		scenery
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef int(DLLGenerateTileSceneryIntf)(SPosition*,SPosition*,int,int,int,int,int,int);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLFilterMetar(
//			char *metarFilename);
//
//	Params:
//		metarFilename	- filename of METAR file
//		
//	Usage:
//		Called just before a METAR file is loaded
//		and processed by Legacy. If you choose to
//		filter the file, you must filter and
//		rewrite the file back to the original
//		filename with all changes in parseable
//		METAR format. 
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLFilterMetarIntf)(char *metarFilename);

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	Function:
//		void DLLInitGlobalMenus(void);
//
//	Params:
//		None
//		
//	Usage:
//		Called just after the global menu bar
//		has been created. Use this routine to
//		install custom menu items into the 
//		global menu bar.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef void(DLLInitGlobalMenusIntf)(void);


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
//	Callback Structure
//
//	The DLLFunctionRegistry structure is passed during
//	the DLLInit call to allow the DLL to populate 
//	function pointers for the functions it implements.
//
//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

#ifdef WIN
#pragma pack(1)
#endif

#ifdef MAC
#pragma options align = packed
#endif

typedef struct DLLFunctionRegistry
{
    DLLTestIntf                 *DLLTest;
	DLLStartSituationIntf		*DLLStartSituation;
	DLLEndSituationIntf		*DLLEndSituation;
	DLLDestroyObjectIntf		*DLLDestroyObject;
	DLLReceiveMessageIntf		*DLLReceiveMessage;
	DLLInstantiateIntf		*DLLInstantiate;
	DLLInitObjectIntf		*DLLInitObject;
	DLLIdleIntf			*DLLIdle;
	DLLDrawIntf			*DLLDraw;
	DLLReadIntf			*DLLRead;
	DLLReadFinishedIntf		*DLLReadFinished;
	DLLWriteIntf			*DLLWrite;
	DLLMouseClickIntf		*DLLMouseClick;
	DLLTrackClickIntf		*DLLTrackClick;
	DLLStopClickIntf		*DLLStopClick;
	DLLMouseMovedIntf		*DLLMouseMoved;
	DLLPrepareIntf			*DLLPrepare;
	DLLTimeSliceIntf		*DLLTimeSlice;
	DLLKeyPressedIntf		*DLLKeyPressed;
	DLLKeyCallbackIntf		*DLLKeyCallback;
	DLLProcessSoundEffectsIntf	*DLLProcessSoundEffects;
	DLLSuspendSoundEffectsIntf	*DLLSuspendSoundEffects;
	DLLResumeSoundEffectsIntf	*DLLResumeSoundEffects;
	DLLInWindowRegionIntf		*DLLInWindowRegion;
	DLLGetPopupHelpIntf		*DLLGetPopupHelp;
	DLLUpdateCameraIntf		*DLLUpdateCamera;
	DLLIsInteriorCameraIntf		*DLLIsInteriorCamera;
	DLLGetCameraNameIntf		*DLLGetCameraName;
	DLLDrawOverlayIntf		*DLLDrawOverlay;
	DLLReadIniFileIntf		*DLLReadIniFile;
	DLLWriteIniFileIntf		*DLLWriteIniFile;
	DLLControlCanBeUsedIntf		*DLLControlCanBeUsed;
	DLLControlButtonCountIntf	*DLLControlButtonCount;
	DLLControlButtonPressedIntf	*DLLControlButtonPressed;
	DLLControlButtonNameIntf	*DLLControlButtonName;
	DLLControlDebounceButtonIntf	*DLLControlDebounceButton;
	DLLControlHasAxisIntf		*DLLControlHasAxis;
	DLLControlGetAxisIntf		*DLLControlGetAxis;
	DLLControlGetIdentifierIntf	*DLLControlGetIdentifier;
	DLLRunScriptIntf		*DLLRunScript;
	DLLCallScriptFunctionIntf	*DLLCallScriptFunction;
	DLLStopScriptIntf		*DLLStopScript;
	DLLStopScriptByRefNameIntf	*DLLStopScriptByRefName;
	DLLGetScriptInfoIntf		*DLLGetScriptInfo;
	DLLBeginFrameIntf		*DLLBeginFrame;
	DLLKeyCallback2Intf		*DLLKeyCallback2;
	DLLGetPythonMethodTableIntf	*DLLGetPythonMethodTable;
	DLLEventNoticeIntf		*DLLEventNotice;
	DLLEpicEventIntf		*DLLEpicEvent;
	DLLGenerateTileSceneryIntf	*DLLGenerateTileScenery;
	DLLFilterMetarIntf		*DLLFilterMetar;
	DLLKeyInterceptIntf		*DLLKeyIntercept;
	DLLInitGlobalMenusIntf		*DLLInitGlobalMenus;

} DLLFunctionRegistry;

#ifdef WIN
#pragma pack()
#endif

#ifdef MAC
#pragma options align = reset
#endif

//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

};	// extern "C"

#endif // __DLLINTF__
