
#include "MainHeader.h"


//#	UI globals

SDLLObject *glss = NULL;

const unsigned int	DESKTOP				= 0;
const unsigned int	GLOBAL_MENUBAR		= 'MBar';
const unsigned int	PLUGINS_MENU_ID		= 'rotw';
#define				PLUGINS_MENU_NAME	  "Plugins"
const unsigned int	MY_MENU_ITEM		= 'YGET'; // to be modified
const unsigned int	WIN_FILE_ID1		= 'aget';
#define				WIN_FILE_NAME1		  "Ui/TEMPLATES/rotw_YAGET_v10_.win"

char				B_SAVE_POS[12]      = { "Save Pos"   };
char				B_SAVE_PATH[12]     = { "Save Path"  };
char				B_PRINT_PATH[12]    = { "Print Path" };
char				B_TRACK_POS[12]     = { "Track Pos"  };
char				B_TRACKING[12]      = { "Tracking.." };
char				B_DISABLED[12]      = { "--------"   };

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
//	Utility 
//
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

unsigned int		    PLUGIN_FLAG		= 0;
const unsigned int	DISABLED			= 0;
const unsigned int	ENABLED	    	= (1 << 0);//1
const unsigned int	SAVE_POINT		= (1 << 1);//2
const unsigned int	GET_PATH			= (1 << 2);//4
const unsigned int	SAVE_PATH			= (1 << 3);//8
const unsigned int	SET_TIMER			= (1 << 4);//16
const unsigned int	TRACKING			= (1 << 5);//32
const unsigned int	LOOP			    = (1 << 6);//64

void TagToString (char* s, unsigned long tag)
{
  s[0] = (char)((tag >> 24) & 0xff);
  s[1] = (char)((tag >> 16) & 0xff);
  s[2] = (char)((tag >> 8) & 0xff);
  s[3] = (char)((tag) & 0xff);
  s[4] = '\0';
}

char* TagToString (unsigned long tag)
{
  static char s[8];
  TagToString (s, tag);
  return s;
}
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
//	
//
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
bool is_window_visible = false;

void AddPathCoordinates  (void);
bool TestDistance        (void);
void Init_File_TrackPath (void);
void StopTracking        (void);
void StartTracking       (void);

void ClosePath           (void);


/////////////////////////////////////////////////////////////
//  Globals
/////////////////////////////////////////////////////////////

int	m_user_timer_value  = 1000;
float distance_control_ = 0.0f;

int   lat_d_ = 0;
float lat_m_ = 0.0;
int   lon_d_ = 0;
float lon_m_ = 0.0;
int   alt_   = 0;
int   galt_  = 0;
int   head_  = 0;
float gSpd_  = 0.0;

float google_tilt_ = 0.0f;
bool tilt_flag_ = false;

#define PI                (3.1415926)
#define RAD2DEG           (180.0 / PI)

my_file::CWriteFileBase *filePath  = NULL;
feop::CFileOp           *trackPath = NULL;

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
//	DLL Function Implementations
//
/////////////////////////////////////////////////////////////

//
//
//	GetUserData
//

SFlyObjectRef user;
float magHeadingDegs;
SPosition userPos;
float userAlt;
float speed;

void GetUserAircraftData()
{
	gRegistry.APIGetUserObject(&user);
	gRegistry.APIGetObjectPosition(&user, &userPos);

	gRegistry.APIGetObjectAGL(&user, &userAlt);
  gRegistry.APIGetObjectSpeed(&user, &speed);
	gRegistry.APIGetObjectMagneticHeading(&user, &magHeadingDegs);

  if (userPos.lon > 648000) userPos.lon += -1296000.0;
  lat_d_ = static_cast<int>    (userPos.lat / 3600.0);
  lat_m_ = static_cast<float> ((userPos.lat - lat_d_ * 3600.0) / 60.0);
  lon_d_ = static_cast<int>    (userPos.lon / 3600.0);
  lon_m_ = static_cast<float> ((userPos.lon - lon_d_ * 3600.0) / 60.0);

  alt_   = static_cast<int> (userPos.alt);

  galt_  = static_cast<int> (userAlt);
  head_  = static_cast<int> (gRegistry.APIWrap360 (magHeadingDegs));
  gSpd_  = speed; // ft/s

}
/////////////////////////////////////////////////////////////
void	DLLIdle(float deltaTimeSecs);

void	DLLDestroyObject(SDLLObject *object)
{

}

void	DLLStartSituation(void)
{
	//
	//	this gets called whenever Fly! begins a
	//	new flight situation.
	//	(Fly Now or Flight Planner Fly Entries)
	//

    //PLUGIN_FLAG |= LOOP;  // lc 110109

    //StartTracking (); // lc 110109

//#ifdef _DEBUG	
	FILE *fp_debug;
	if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	{
		int test = 0;
		fprintf(fp_debug, "DLLStartSituation %s %d\n", "GOOGLE EARTH DLL", PLUGIN_FLAG);
		fclose(fp_debug); 
	}
//#endif

}

void	DLLEndSituation(void)
{
	//
	//	This gets called whenever Fly! ends a
	//	flight situation.
	//	You should cleanup anything you added
	//	to Fly!
	//

}

//#################################################
//#
//#	InitGlobalMenus
//#
//#	Implement this Python routine to install
//#	customizations into the global (primary)
//#	Fly! menubar
//#
//#################################################

void DLLInitGlobalMenus(void)
{
//#
//# add a Plugins menu (Fly! will ignore this request if a Plugins menu already exists)
//# APIAddUIMenu(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID, PLUGINS_MENU_NAME)

	gRegistry.APIAddUIMenu(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID, PLUGINS_MENU_NAME);

//# 
//# add a menu item to the Plugins menu to open our custom window
//# APIAddUIMenuItem(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID, SIMPLE_VIEW_WINDOW, "SimpleView Sample")

	gRegistry.APIAddUIMenuItem(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID, MY_MENU_ITEM, "YAGET");

    // add a new folder
//    char command[64] = {'\0'};
//    sprintf (command, "mkdir GoogleEarth");
//    system (command);
}

//#################################################
//#
//#	EventNotice
//#
//#	Implement this routine to get
//#	notified on menu and window events
//#
//#################################################

void DLLEventNotice(SDLLObject *notify,	unsigned long windowID, unsigned long componentID, unsigned long event, unsigned long subEvent)
{
//#
//# handle menu item selection events
//#---------------------------------------------
  #ifdef _DEBUG	
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
	  {
      char buf1[128] = {0};
      char buf2[128] = {0};
      char buf3[128] = {0};
      char buf4[128] = {0};
      TagToString (buf1, windowID);
      TagToString (buf2, componentID);
      TagToString (buf3, event);
      TagToString (buf4, subEvent);
		  fprintf(fp_debug, "DLLEventNotice 1=%s 2=%s 3=%s 4=%s\n",
        buf1, buf2, buf3, buf4);
		  fclose(fp_debug); 
	  }
  #endif

	if (event == EVENT_SELECTITEM)
	{
		if (componentID == PLUGINS_MENU_ID)
		{
			if (subEvent == MY_MENU_ITEM)
			{
        #ifdef _DEBUG	
	        FILE *fp_debug;
	        if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
	        {
		        fprintf(fp_debug, "PLUGIN_FLAG %d open?%d visible?%d\n",
              PLUGIN_FLAG, 
              gRegistry.APIIsWindowOpen(WIN_FILE_ID1),
              is_window_visible);
		        fclose(fp_debug); 
	        }
        #endif

			  if(!PLUGIN_FLAG) 
        {
//			  # only allow one window to be opened at a time
				  if (!gRegistry.APIIsWindowOpen(WIN_FILE_ID1))
          {
            #ifdef _DEBUG	
	            FILE *fp_debug;
	            if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
	            {
                 char buf1[128] = {0};
                 TagToString (buf1, WIN_FILE_ID1);
                 fprintf(fp_debug, "APICreateWindow2 %s %s\n", buf1, WIN_FILE_NAME1);
		            fclose(fp_debug); 
	            }
            #endif
//			    # when the user selects our menu item, open our custom window,
//			    # and place a checkmark on the menu item. 
						// Create UI window
						// calling our UI/Template   WIN_FILE_NAME  custom model
				    glss = gRegistry.APICreateDLLObject(NULL);	// according tonycrider forum advise
				    glss->dllObject = 0;			// according tonycrider forum advise
				    gRegistry.APICreateWindow2(WIN_FILE_ID1, WIN_FILE_NAME1, glss);
//					  gRegistry.APICheckUIMenuItem(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID, MY_MENU_ITEM, 1);
            is_window_visible = true;
            gRegistry.APISetUIButtonLabel    (WIN_FILE_ID1, 'btn3', B_DISABLED);
            gRegistry.APISetUIButtonLabel    (WIN_FILE_ID1, 'btn4', B_DISABLED);
            gRegistry.APISetUIButtonLabel    (WIN_FILE_ID1, 'btn5', B_DISABLED);
            gRegistry.APISetUIButtonLabel    (WIN_FILE_ID1, 'btn6', B_DISABLED);
            //gRegistry.APISetUIScrollbarValue (WIN_FILE_ID1, 'scb1', google_tilt_ * 100.0f / 70.0f);
          }
          PLUGIN_FLAG = ENABLED;
        }
		    else 
        {
			    if (gRegistry.APIIsWindowOpen(WIN_FILE_ID1))
          {
            if (is_window_visible == false) {
              gRegistry.APIShowWindow(WIN_FILE_ID1);
              is_window_visible = true;
              #ifdef _DEBUG	
                FILE *fp_debug;
                if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
                  {
                    fprintf(fp_debug, "else show PLUGIN_FLAG %d open?%d visible?%d\n",
                      PLUGIN_FLAG, gRegistry.APIIsWindowOpen(WIN_FILE_ID1), is_window_visible);
                    fclose(fp_debug); 
                  }
              #endif
            } else{
              gRegistry.APIHideWindow(WIN_FILE_ID1);
              is_window_visible = false;
              #ifdef _DEBUG	
                FILE *fp_debug;
                if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
                {
                  fprintf(fp_debug, "else hide PLUGIN_FLAG %d open?%d visible?%d\n", 
                    PLUGIN_FLAG, gRegistry.APIIsWindowOpen(WIN_FILE_ID1), is_window_visible);
                  fclose(fp_debug); 
                }
              #endif
            }
          }
        }
			}
		}
	}
//#------------------------------------------------------------------------------------
//#
//# 
//#------------------------------------------------------------------------------------
	else if (event == EVENT_CLOSEWINDOW)
	{
		if (windowID == WIN_FILE_ID1)
		{
//    # when our custom window is closed, uncheck the menu item
//		# and re-enable the menu item so it can be selected again
////	APICheckUIMenuItem(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID, WIN_FILE_ID1, 0);
////	PLUGIN_FLAG = DISABLED;
			if(glss) {
        glss = NULL;
        gRegistry.APICloseWindow2(WIN_FILE_ID1);
        #ifdef _DEBUG	
	      { FILE *fp_debug;
	        if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
	        {
            char buf1[128] = {0};
            TagToString (buf1, WIN_FILE_ID1);
		        fprintf(fp_debug, "APICloseWindow2 %s %d\n", buf1, PLUGIN_FLAG);
		        fclose(fp_debug); 
	        }
        }
        #endif
      }
////  PLUGIN_FLAG &= ~GET_PATH;
////  PLUGIN_FLAG &= ~TRACKING;
      ClosePath ();
      StopTracking ();
			PLUGIN_FLAG = DISABLED;
//		APICheckUIMenuItem(DESKTOP, GLOBAL_MENUBAR, PLUGINS_MENU_ID, MY_MENU_ITEM, 0);
    }
	}
	//
	else if (windowID == WIN_FILE_ID1 && componentID == 'btn1')
	{
   	if (event == EVENT_BUTTONPRESSED)
		{
       if (PLUGIN_FLAG & LOOP) {
         PLUGIN_FLAG &= ~LOOP;
         #ifdef _DEBUG	
	         FILE *fp_debug;
	         if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
	         {
		         fprintf(fp_debug, "Stop Looping\n");
		         fclose(fp_debug); 
	         }
         #endif
         gRegistry.APISetUIButtonLabel (WIN_FILE_ID1, 'btn3', B_DISABLED);
         gRegistry.APISetUIButtonLabel (WIN_FILE_ID1, 'btn4', B_DISABLED);
         gRegistry.APISetUIButtonLabel (WIN_FILE_ID1, 'btn5', B_DISABLED);
         gRegistry.APISetUIButtonLabel (WIN_FILE_ID1, 'btn6', B_DISABLED);
       } else { 
         PLUGIN_FLAG |= LOOP;
         #ifdef _DEBUG	
	         FILE *fp_debug;
	         if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
	         {
		         fprintf(fp_debug, "Start Looping\n");
		         fclose(fp_debug); 
	         }
         #endif
//       APISetUIButtonLabel (WIN_FILE_ID1, 'btn3', B_SAVE_POS);
//       APISetUIButtonLabel (WIN_FILE_ID1, 'btn4', B_SAVE_PATH);
//       APISetUIButtonLabel (WIN_FILE_ID1, 'btn5', B_DISABLED);
         gRegistry.APISetUIButtonLabel (WIN_FILE_ID1, 'btn6', B_TRACK_POS);
//       APISetUIScrollbarValue (WIN_FILE_ID1, 'scb1', 0.0f);
       }
//
////   PLUGIN_FLAG &= ~GET_PATH;
////   PLUGIN_FLAG &= ~TRACKING;
       ClosePath ();
       StopTracking ();
       PLUGIN_FLAG &= ~SAVE_PATH;
    }
	}

	else if (windowID == WIN_FILE_ID1 && componentID == 'btn2')
	{
   	if (event == EVENT_BUTTONPRESSED)
		{
          char buff[64] = {'\0'};
          strncpy (buff, gRegistry.APIGetUITextText(WIN_FILE_ID1, 'edt1'), 64);
          int val = static_cast<int> (atof (buff) * 1000);
          #ifdef _DEBUG
          {
	          FILE *fp_debug;
	          if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
            {
		          fprintf(fp_debug, "Timer \"%s\" %d\n", buff, val);
		          fclose(fp_debug); 
            }
          }
          #endif
          if (val > 0 && val < 5001)
            m_user_timer_value = val;
    }
	}
//
  else if (windowID == WIN_FILE_ID1 && componentID == 'btn3')
	{
   	if (event == EVENT_BUTTONPRESSED)
		{
//          if (!strcmp (APIGetUIButtonLabel (WIN_FILE_ID1, 'btn3'), B_SAVE_POS)) {
//            my_file::SUserPos pos;
//            pos.lat  = userPos.lat / 3600.0;
//            pos.lon  = userPos.lon / 3600.0;
//            pos.alt  = userPos.alt;
//            pos.ga   = userAlt;
//            if (pos.ga < 0) pos.ga = 5;
//            pos.head = static_cast<int> (APIWrap360 (magHeadingDegs));
//            pos.spd  = speed;
//
//            my_file::CWriteFileBase *file = new my_file::CWritePosition (".\\GoogleEarth\\test_", pos);
//            if (file) {file->Write (); delete (file); file = NULL;}
//          
//	            ///////////////// remove later ////////////////////////////////
//	            char buffer[128] = {0};
//	            sprintf(buffer, "Position Saved in ..Fly! II\\GoogleEarth\\test_.kml file");
//	            APIDrawNoticeToUser(buffer,5);
//	            ///////////////// remove later ////////////////////////////////
//          }   
    }
	}
	
  else if (windowID == WIN_FILE_ID1 && componentID == 'btn4')
	{
   	if (event == EVENT_BUTTONPRESSED)
		{
//          if (!strcmp (APIGetUIButtonLabel (WIN_FILE_ID1, 'btn4'), B_SAVE_PATH)) {
//            
//            distance_control_ = 0.0f;
//
//            if (filePath = new my_file::CWritePath (".\\GoogleEarth\\test_path")) {
//
//              PLUGIN_FLAG |= GET_PATH;
//              PLUGIN_FLAG &= ~SAVE_PATH;
//              APISetUIButtonLabel (WIN_FILE_ID1, 'btn4', B_DISABLED);
//              APISetUIButtonLabel (WIN_FILE_ID1, 'btn5', B_PRINT_PATH);
//            } else {
//              PLUGIN_FLAG &= ~GET_PATH;
//              PLUGIN_FLAG &= ~SAVE_PATH;
//              APISetUIButtonLabel (WIN_FILE_ID1, 'btn4', B_DISABLED);
//              APISetUIButtonLabel (WIN_FILE_ID1, 'btn5', B_DISABLED);
//
//	          ///////////////// remove later ////////////////////////////////
//	          char buffer[128] = {0};
//	          sprintf(buffer, "%s", "Can't open file !");
//	          APIDrawNoticeToUser(buffer,5);
//	          ///////////////// remove later ////////////////////////////////
//            }
//          }
    }
	}
	
  else if (windowID == WIN_FILE_ID1 && componentID == 'btn5')
	{
   	if (event == EVENT_BUTTONPRESSED)
		{
//          if (!strcmp (APIGetUIButtonLabel (WIN_FILE_ID1, 'btn5'), B_PRINT_PATH)) {
//            
//            ClosePath ();
//          }
    }
	}

  else if (windowID == WIN_FILE_ID1 && componentID == 'btn6')
	{
   	if (event == EVENT_BUTTONPRESSED)
		{
      char buff [128] = {0};
      strncpy (buff, gRegistry.APIGetUIButtonLabel (WIN_FILE_ID1, 'btn6'), 128);
      if (!strcmp (buff, B_TRACK_POS)) {

        StartTracking ();
/*
        #ifdef _DEBUG
        {
	        FILE *fp_debug;
	        if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
          {
		        fprintf(fp_debug, "StartTracking \"%s\"\n", B_TRACK_POS);
		        fclose(fp_debug); 
          }
        }
        #endif
*/
      }
      else 
      if (!strcmp (buff, B_TRACKING)) {
            
        StopTracking ();
/*
        #ifdef _DEBUG	
        {FILE *fp_debug;
           if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
           {
             fprintf(fp_debug, "StopTracking\n");
             fclose(fp_debug); 
           }
        }
        #endif
*/
      }
    }
	}
//
  else if (windowID == WIN_FILE_ID1 && componentID == 'scb1')
  {
//		{
//	      float tmp = APIGetUIScrollbarValue(WIN_FILE_ID1, 'scb1') * 70.0f;
//          google_tilt_ = tmp / 100.0f;
//          tilt_flag_ = true;
//          #ifdef _DEBUG	
//	          ///////////////// remove later ////////////////////////////////
//	          char buffer[128] = {0};
//	          sprintf(buffer, "slider %05.2f %02.0f", tmp, google_tilt_);
//	          APIDrawNoticeToUser(buffer,1);
//	          ///////////////// remove later ////////////////////////////////
//          #endif
//    }
	}

}

/**************************************************************************************/

int DLLRead(SDLLObject *object, SStream *stream, unsigned int tag)
{
	int result = TAG_READ;

	switch(tag)
	{
    case 1:
	default:
		result = TAG_IGNORED;
	}
	return(result);
}

/////////////////////////////////////////////////////////////
//
//	DLL Setup (Init & Kill)
//
//	These functions MUST be implemented, or the DLL
//	will totally fail!!!!
//
/////////////////////////////////////////////////////////////

DLL_INTERFACE int DLLInit(DLLFunctionRegistry *dll, SDLLCopyright *copyright, SDLLRegisterTypeList **types)
{
	//
	//	populate copyright info
	//

	strcpy(copyright->product, "Legacy Google Earth dll");
	strcpy(copyright->company, "ROTW (c)");
	strcpy(copyright->programmer, "laurentC");
	strcpy(copyright->dateTimeVersion, "1.00, 10/08/06 2005Z");
	strcpy(copyright->email, "no@email.com");
	copyright->internalVersion = 100;

	//
	//	return function pointers of available DLL functions
	//
//dll->DLLInstantiate			= DLLInstantiate;
//dll->DLLDestroyObject		= DLLDestroyObject;
	dll->DLLStartSituation	= DLLStartSituation;
//dll->DLLEndSituation		= DLLEndSituation;
//dll->DLLRead				    = DLLRead;
	dll->DLLEventNotice		  = DLLEventNotice;
	dll->DLLInitGlobalMenus	= DLLInitGlobalMenus;
	dll->DLLIdle				    = DLLIdle;

	//
	//	Init Buddy
	//



	//
	//	result code
	//

	//
	//	get dsk api from application
	//
    GetSDKAPI (gRegistry);

	return(1);
}

void	DLLKill(void)
{
	//
	//	nothing mandatory to do here
	//	except internal cleanup
	//

    ClosePath ();
    // no gRegistry anymore
    StopTracking ();

  #ifdef _DEBUG	
  {	FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "==DLLKill %s\n", "Legacy Google Earth dll");
		  fclose(fp_debug); 
  }	}
  #endif
}

void AddPathCoordinates (void)
{
  if (TestDistance ()) {
    my_file::SUserPos pos;

            pos.lat  = userPos.lat / 3600.0;
            pos.lon  = userPos.lon / 3600.0;
            pos.alt  = userPos.alt;
            pos.ga   = userAlt;
            if (pos.ga < 0) pos.ga = 5;
            pos.head = static_cast<int> (/*APIWrap360*/ (magHeadingDegs));
            pos.spd  = speed;

    filePath->AddCoordinates (pos);
  }
}

bool TestDistance (void) 
{
  distance_control_ += gSpd_ * (m_user_timer_value / 1000.0f);

  #ifdef _DEBUG	
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "distance_control_ %f %f\n", gSpd_, distance_control_);
		  fclose(fp_debug); 
	  }
  #endif
    
  if (distance_control_ > 6077.0f) {
    distance_control_ -= 6077.0f;
    return true;
  }
  return false;
}

void Init_File_TrackPath (void) 
{
  my_file::CWriteTrackPath *trackInitPath = NULL;

  if (!trackInitPath) {

    my_file::SUserPos pos;
    pos.lat  = lat_d_ + (lat_m_ / 60.0);
    pos.lon  = lon_d_ + (lon_m_ / 60.0);
    pos.alt  = static_cast<double>(alt_);
    pos.ga   = pos.alt - static_cast<double>(galt_);
    if (pos.ga < 0) pos.ga = 5;
    pos.head = head_;
    pos.spd  = static_cast<double>(gSpd_);

    //

    if (trackInitPath = new my_file::CWriteTrackPath (".\\GoogleEarth\\test_track_path", pos)) {
      PLUGIN_FLAG |=  TRACKING;
    }
    else {
      PLUGIN_FLAG &= ~TRACKING;
    }

    #ifdef _DEBUG	
    if (PLUGIN_FLAG & TRACKING) {
	    FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
	    {
		    fprintf(fp_debug, "CWriteTrackPath : constructor %d\n", trackInitPath);
		    fclose(fp_debug); 
	    }
    }
    #endif
  }

  if (trackInitPath) {
    // write
    trackInitPath->Write ();

    delete (trackInitPath);

    #ifdef _DEBUG	
	    FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
      {
  		  fprintf(fp_debug, "CWriteTrackPath : destructor  %d\n", trackInitPath);
	  	  fclose(fp_debug); 
      }
    #endif 
    
    trackInitPath = NULL;

    // file elementary operations
    trackPath = new feop::CFileOp (".\\GoogleEarth\\test_track_path.kml");
  }
}

void StopTracking (void)
{
  if (PLUGIN_FLAG & TRACKING) {

    PLUGIN_FLAG &= ~TRACKING;
    if (trackPath) trackPath->StopOp ();
    remove (".\\GoogleEarth\\test_track_path.kml");
    if (PLUGIN_FLAG & LOOP)
      gRegistry.APISetUIButtonLabel (WIN_FILE_ID1, 'btn6', B_TRACK_POS);

  ///////////////// remove later ////////////////////////////////
	//char buffer[128] = {0};
	//sprintf(buffer, "%s", "Stop Tracking ...");
	//gRegistry.APIDrawNoticeToUser(buffer,5);
	///////////////// remove later ////////////////////////////////
    
    #ifdef _DEBUG	
	    FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
	    {
		    int test = 0;
		    fprintf(fp_debug, "StopTracking\n");
		    fclose(fp_debug); 
	    }
    #endif

  }
}

void StartTracking (void)
{
  if (!(PLUGIN_FLAG & TRACKING)) {

    PLUGIN_FLAG |= TRACKING;
    Init_File_TrackPath ();
    gRegistry.APISetUIButtonLabel (WIN_FILE_ID1, 'btn6', B_TRACKING);

  ///////////////// remove later ////////////////////////////////
	//char buffer[128] = {0};
	//sprintf(buffer, "%s", "Tracking ...");
	//gRegistry.APIDrawNoticeToUser(buffer,5);
	///////////////// remove later ////////////////////////////////

    #ifdef _DEBUG	
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "StartTracking %s\n", B_TRACKING);
		  fclose(fp_debug); 
	  }
    #endif 

  }
}

void ClosePath (void)
{
  if (PLUGIN_FLAG & GET_PATH) {
    PLUGIN_FLAG |= SAVE_PATH;
    PLUGIN_FLAG &= ~GET_PATH;
    distance_control_ = 0.0f;

    filePath->Write ();

    if (filePath) {
      delete (filePath);
      filePath = NULL;
    }

  //              APISetUIButtonLabel (WIN_FILE_ID1, 'btn4', B_SAVE_PATH);
  //              APISetUIButtonLabel (WIN_FILE_ID1, 'btn5', B_DISABLED);

  #ifdef _DEBUG	
    FILE *fp_debug;
    if(!(fp_debug = fopen("__DDEBUG_legacy_google.txt", "a")) == NULL)
    {
	    int test = 0;
	    fprintf(fp_debug, "ClosePath\n");
	    fclose(fp_debug); 
    }
  #endif 

  }
}