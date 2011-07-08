/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
//	.CPP
//
//
/////////////////////////////////////////////////////////////

#include "extrafile.h"


/////////////////////////////////////////////////////////////
//  Globals
/////////////////////////////////////////////////////////////

    static int timer  = 0;

/////////////////////////////////////////////////////////////
//  Declarations
/////////////////////////////////////////////////////////////
    void PrintTextInfo   (void);
    void PrintTimerInfo  (void);
    void Print_TrackPath (void);

/////////////////////////////////////////////////////////////


//
//	DLLIdle
//

void	DLLIdle(float deltaTimeSecs)
{

  if (PLUGIN_FLAG) {
	if (PLUGIN_FLAG & LOOP) // save a point
	{
      timer += static_cast<int> (deltaTimeSecs * 1000);
      if (timer > m_user_timer_value) {
        GetUserAircraftData ();
        PrintTextInfo ();

        timer -= m_user_timer_value;

//	    if(PLUGIN_FLAG & GET_PATH) // record a track
//	    {
//          AddPathCoordinates ();
//	    }
//        else if(PLUGIN_FLAG & SAVE_PATH)
//	    {
//	    }

	    if(PLUGIN_FLAG & TRACKING) // track
	    {
          Print_TrackPath ();
        }
      }
    }
    else {
      PrintTimerInfo ();
    }
  }

}

void PrintTextInfo (void)
{/*
  #ifdef _DEBUG	
  FILE *fp_debug;
  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
  {
	  int test = 0;
	  fprintf(fp_debug, "GOOGLE EARTH-DLLIdle %d\n");
	  fclose(fp_debug); 
  }
  #endif
*/
        char buff[256] = {'\0'};

        sprintf(buff, " Lat = %d %.3f \t Lon = %d %.3f \t Alt = %d",
                      lat_d_ ,
                      lat_m_ ,
                      lon_d_ ,
                      lon_m_ ,
                      alt_   );
        gRegistry.APISetUITextText(WIN_FILE_ID1, 'lbl2', buff);
        sprintf(buff, " GA  = %03d \t Head = %03d  \t Timer = %d ms",
                      galt_  ,
                      head_  ,
                      m_user_timer_value);
        gRegistry.APISetUITextText(WIN_FILE_ID1, 'lbl3', buff);
        sprintf(buff, " spd = %.2f ft/s \t tilt = %02.0f d",
                      gSpd_,
                      google_tilt_);
        gRegistry.APISetUITextText(WIN_FILE_ID1, 'lbl4', buff);

}

void PrintTimerInfo (void)
{
/*
        char buff[512] = {'\0'};
       
        sprintf(buff, "Timer = %d ms", m_user_timer_value);

      APISetUITextText(WIN_FILE_ID1, 'edt2', buff);
*/
}


void Print_TrackPath (void) 
{

  // file elementary operations
  if (trackPath) {
   if (trackPath->IsSafeOp ())
   {
      char bufflat10[10] = {'\0'};
      char bufflon10[10] = {'\0'};
      char buffhead03[3] = {'\0'};

      sprintf (bufflat10, "%010.5f", lat_d_ + (lat_m_ / 60.0));
      sprintf (bufflon10, "%010.5f", lon_d_ + (lon_m_ / 60.0));
      sprintf (buffhead03,   "%03d", head_);

      trackPath->WriteBufFromStart (bufflon10, 387L);
      trackPath->WriteBufFromPos   (bufflat10,   1L);
      trackPath->WriteBufFromPos   (bufflon10,  76L);
      trackPath->WriteBufFromPos   (bufflat10,  27L);
      trackPath->WriteBufFromPos   (buffhead03,  25L);

//      char buff10[64] = {'0'};
//      char buff03[64] = {'0'};
//      strncpy (buff10, "----------", 10);
//      strncpy (buff03, "+++", 3);
//
//      trackPath->WriteBufFromStart (buff10, 387L);
//      trackPath->WriteBufFromPos   (buff10,   1L);
//      trackPath->WriteBufFromPos   (buff10,  76L);
//      trackPath->WriteBufFromPos   (buff10,  27L);
//      trackPath->WriteBufFromPos   (buff03,  25L);

      if (tilt_flag_) {
        tilt_flag_ = false;
        char buffhead05[5] = {'\0'};
        sprintf (buffhead05, "%05.2f", google_tilt_);
        trackPath->WriteBufFromPos   (buffhead05, 24L);
      }
      // removed 091606
//      trackPath->StopOp  ();
//      trackPath->StartOp ();
    }
  }

}

