// main.cpp : Defines the entry point for the DLL application.
//

#include "main.h" // FILE

/////////////////////////////////////////////////////////////
//
//	GetUserData
//
/////////////////////////////////////////////////////////////



SFlyObjectRef user;
SPosition userPos;
SVector ori;
float speed = 0.0f;

void GetUserAircraftData()
{
	gRegistry.APIGetUserObject(&user);
	gRegistry.APIGetObjectPosition(&user, &userPos);
  gRegistry.APIGetObjectOrientation(&user, &ori);
  gRegistry.APIGetObjectSpeed(&user, &speed);
}


const double PI      = 3.14159265358979323846;   /* From M_PI under Linux/X86 */
const double RAD2DEG = (180.0 / PI);
////////////////////////////////////////////////////////////
const int MaxBuds = 2;
struct BuddyInfo 
{
  SFlyObjectRef *fly;

} buddy [MaxBuds];

SDLLObject *obj [MaxBuds];

static int obj_counter = 0;

//************************************************************
//*                         MoveBuddy                         *
//************************************************************
void MoveBuddy (BuddyInfo *info, const float &speed = 0.0f, const int &num = 0)
{
  if (info == NULL) return;
  if (info->fly == NULL) return;

  SFlyObjectRef *bud = info->fly;
  if (num == 0) {
    ;
  }
  float spd = speed;
  gRegistry.APISetObjectSpeed (bud, &spd);

#ifdef _DEBUG_
  SPosition pos;
  gRegistry.APIGetObjectPosition (bud, &pos);
  SVector ori;
  gRegistry.APIGetObjectOrientation (bud, &ori);

  char buff [1024] = {'\0'};
  sprintf (buff, "%.2f %.2f %.2f %.2f %.2f %.2f\n",
       pos.lat, pos.lon, pos.alt,
       ori.x, ori.y, ori.z);
  gRegistry.APIDrawNoticeToUser (buff, 1);
#endif
}

//************************************************************
//*                         PutBuddy                         *
//************************************************************
static double tim = 0.0;
void PutBuddy (BuddyInfo *info, const int &num = 0)
{
  if (info == NULL) return;
  if (info->fly == NULL) return;

  SFlyObjectRef *bud = info->fly;

//  SVector iRelVector;
//  iRelVector.x =   010.0;
//  iRelVector.y =  -100.0;
//  iRelVector.z =   000.0;
//  SPosition buddyPos = gRegistry.APIAddVector (&userPos, &iRelVector);

  SPosition buddyPos;
  if (num == 0) {
  buddyPos.lat = 177697.26;
  buddyPos.lon = 575.89;
  buddyPos.alt = 466.0;
  } else
  if (num == 1) {
  buddyPos.lat = 177697.26;
  buddyPos.lon = 574.89;
  buddyPos.alt = 466.0;
  } 
  gRegistry.APISetObjectPosition (bud, &buddyPos);

  SVector buddyOr;
  buddyOr.x = 0.0;
  buddyOr.y = 0.0;
  buddyOr.z = (tim - 90.0) / RAD2DEG;
  tim += 0.1;
  if (tim > 360.0) tim = 0.1;
  gRegistry.APISetObjectOrientation (bud, &buddyOr);
}

//************************************************************
//*                    DLLStartSituation                     *
//************************************************************
void DLLStartSituation (void)
{
  obj_counter = 0;

  obj [0] = gRegistry.APICreateFlyObject ("sobj_cessna.sit");
  if (obj [0] != NULL) {
    obj_counter++;
    buddy [0].fly = &(obj [0]->flyObject);
    PutBuddy (&buddy [0], 0);
  }

  obj [1] = NULL;
  obj [1] = gRegistry.APICreateFlyObject ("sobj_cessna.sit");
  if (obj [1] != NULL) {
    obj_counter++;
    buddy [1].fly = &(obj [1]->flyObject);
    PutBuddy (&buddy [1], 1);
  }

//#ifdef _DEBUG
    time_t dateTime;
    time (&dateTime);
    char buff [256];
    sprintf (buff, "1.00, %s", ctime (&dateTime));
    buff[strlen (buff) - 1] = '\0';
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	  {
		  fprintf(fp_debug, "DLLStartSituation %s %s %d\n",
        DLL_NAME, buff, obj_counter);
		  fclose(fp_debug); 
	  }
//#endif

}

//************************************************************
//*                    DLLEndSituation                       *
//************************************************************
void DLLEndSituation (void)
{
  for (int i = 0; i < obj_counter; ++i) {
    if (obj [i] != NULL) {
      #ifdef _DEBUG
        FILE *fp_debug;
	      if(!(fp_debug = fopen("__DDEBUG_dll.txt", "a")) == NULL)
	      {
		      fprintf(fp_debug, "==DLLEndSituation %s [%p]\n", DLL_NAME, obj [i]);
		      fclose(fp_debug); 
	      } 
      #endif
      gRegistry.APIDestroyObject (obj [i]);
      obj [i] = NULL;
    }
  }
}

//************************************************************
//*                         DLLIdle                          *
//************************************************************
void DLLIdle (float deltaTimeSecs)
{
  GetUserAircraftData ();
  for (int i = 0; i < obj_counter; ++i)
    MoveBuddy (&buddy [i], 12.0f, i);
    //PutBuddy (&buddy [i], i);
}

