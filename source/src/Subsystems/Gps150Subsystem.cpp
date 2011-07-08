/*
 * Gps150Subsystems.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2007 Chris Wallace
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

using namespace std;
/*
static const int CrsrNavCdiIdent = 1;
static const int CrsrNavCdiGs = 2;
static const int CrsrNavCdiDistance = 3;
static const int CrsrNavCdiSteer = 4;
static const int CrsrNavCdiEte = 5;

static const int CrsrNavPosProx     = 6;
static const int CrsrNavPosProxWpt1 = 7;
static const int CrsrNavPosProxWpt2 = 8;
static const int CrsrNavPosProxWpt3 = 9;
static const int CrsrNavPosProxWpt4 = 10;

static const int CrsrNavMenu1TripPlan  = 20;
static const int CrsrNavMenu1DaltTas   = 21;
static const int CrsrNavMenu1FuelPlan  = 22;
static const int CrsrNavMenu1Winds     = 23;
static const int CrsrNavMenu1VnavPlan  = 24;
static const int CrsrNavMenu1Chklist   = 25;

static const int CrsrNavMenu2ApprTime  = 30;
static const int CrsrNavMenu2Clock     = 31;
static const int CrsrNavMenu2TripTime  = 32;
static const int CrsrNavMenu2GpsPlan   = 33;
static const int CrsrNavMenu2Scheduler = 34;
static const int CrsrNavMenu2Sunrise   = 35;

static const int CrsrNavAutoStoreId    = 40;
static const int CrsrNavAutoStoreRte   = 41;
static const int CrsrNavAutoStoreOk    = 42;

static const int CrsrNavSchedulerName     = 50;
static const int CrsrNavSchedulerTime     = 51;
static const int CrsrNavSchedulerNameEdit = 52;
static const int CrsrNavSchedulerTimeEdit = 53;

static const int CrsrApprTimeDir       = 60;
static const int CrsrApprTimeHours     = 61;
static const int CrsrApprTimeMinTens   = 62;
static const int CrsrApprTimeMinOnes   = 63;
static const int CrsrApprTimeSecTens   = 64;
static const int CrsrApprTimeSecOnes   = 65;
static const int CrsrApprTimeStart     = 66;
static const int CrsrApprTimeStop      = 67;
static const int CrsrApprTimeReset     = 68;

static const int CrsrClockTimeMode     = 70;
static const int CrsrClockLclHour      = 71;
static const int CrsrClockLclMin       = 72;

static const int CrsrSunriseWpt        = 80;
static const int CrsrSunriseYear       = 81;
static const int CrsrSunriseMonth      = 82;
static const int CrsrSunriseDay        = 83;


//
// Utility function to calculate local date/time, given UTC date/time and local offset
//
void calcLclDateTime (SFsimDate utcDate, SFsimTime utcTime,
					  double lclOffset,
					  SFsimDate &lclDate, SFsimTime &lclTime)
{
	double j = julianDayNumber (utcDate, utcTime);

	// Adjust julian day (in days) by lclOffset (in hours)
	j += lclOffset / 24;

	calendarDate (j, lclDate, lclTime);
}

void Nav::updateNavCdiDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	SGps150RealTimeData *pRealTime = (SGps150RealTimeData *)pData->pRealTime;
	if (!pRealTime) return;

	// Define variables for to/from and user data
	SRteWaypoint wpFrom, wpTo;
	SFsimPosition posFrom, posTo, posUser;

	// Set user position
	posUser = pData->pRealTime->userPos;

	// Calculate nav data
	float dis = 0;
	float brg = 0;
	float dtk = 0;
	float cts = 0;
	float obs = 0;
	float trn = 0;
	float ete = 0;
	float eta = 0;
	float vn = 0;

	// Shortcut pointer to active route
	SRteActive *pRte = &pData->rteActive;
	SRteLeg *pLeg = &pRte->leg[pRte->iLeg];

	// There must be a valid active RTE
	if (pRte->valid) {
		// Set leg endpoints
		wpFrom = pRte->stored.waypoint[pRte->iLeg];
		wpTo   = pRte->stored.waypoint[pRte->iLeg+1];
		posFrom = wpFrom.pos;
		posTo = wpTo.pos;
	} else {
		// No valid route, set all endpoints to user position
		posFrom = posUser;
		posTo = posUser;
	}

	// Calculate great-circle vector between FROM waypoint and USER
	SFsimVector v;
	v = fsimGreatCirclePolar (posFrom, posUser);
	float disFrom = FeetToNm ((float)v.z); // FROM waypont --> USER
	float thetaFrom = (float)v.y;
	float brgFrom = RadToDeg (thetaFrom); // Bearing from FROM wpt

	// Calculate great-circle vector between USER and TO waypoint
	v = fsimGreatCirclePolar (posUser, posTo);
	float disTo = FeetToNm ((float)v.z); // USER --> TO waypoint
	float thetaTo = (float)v.y;
	float brgTo = RadToDeg (thetaTo); // Bearing from USER

	// Distance is always distance from user to TO waypoint
	dis = disTo;

	// Bearing is always bearing from user to TO waypoint
	brg = brgTo;

	// Desired-track
	dtk = pRte->leg[pRte->iLeg].dtk;

	// Calculate cross-track error in NM
	float thetaLeg = DegToRad (dtk);
	float thetaError = (float)(thetaFrom - thetaLeg);
	WrapTwoPi (thetaError);
	float trackError = (float)-sin(thetaError) * disFrom;

	// Calculate CDI scale in NM.  The cdiScale and navUnits
	//   settings (both settable on the SET pages) determine the
	//   CDI sensitivity
	float scale;
	switch (pData->navUnits) {
	case NavNmKt:
		scale = pData->cdiScale;
		break;

	case NavMiMph:
		// Convert CDI scale in Miles to NM
		scale = MiToNm (pData->cdiScale);
		break;

	case NavKmKph:
		// Convert CDI scale in KM to NM
		scale = KmToNm (pData->cdiScale);
		break;
	}

	// Calculate CDI deflection in clamped range -1.0 - +1.0
	float cdiDeflect = trackError / scale;
	if (cdiDeflect > 1.0) cdiDeflect = 1.0;
	else if (cdiDeflect < -1.0) cdiDeflect = -1.0;

	// Set steer fields, and convert track error to absolute value
	char cSteer;
	if (trackError < 0) {
		// Negative cross-track error indicates user should steer L
		cSteer = 'L';
		trackError = -trackError;
	} else {
		// Postive error indicates user should steer R
		cSteer = 'R';
	}

	// Calculate ete in hours (NM / KTS)
	ete = pLeg->legDistance / pRealTime->gs;
	
	// TODO: Add ete to current time for eta
	eta = ete;

	// TODO: Convert ETA into local time if necessary
	if (!pData->useUTC) {
	}

	//
	// Format the display lines
	//

	// Line 1, active waypoint, groundspeed
	char sId[12];
	if (pData->rteActive.valid) {
		sprintf (sId, "%5s}%-5s", wpFrom.name, wpTo.name);
	} else {
		// No waypoint
		strcpy (sId, "_____}_____");
	}

	char sGs[10];
	memset (sGs, 0, 10);
	formatSpeed (pRealTime->gs, pData->navUnits, sGs);

	sprintf (pShared->line1, "%-11s  gs%6s", sId, sGs);

	// Line 2, distance/track
	char sDistanceMode[10];
	char sDistance[10];
	switch (disMode) {
	case NAV_DIS_DISTANCE:
		// Distance mode
		strcpy (sDistanceMode, "dis");
		formatDistance (dis, pData->navUnits, sDistance);
		break;

	case NAV_DIS_STEER:
		{
			// Steer mode
			strcpy (sDistanceMode, "str");
			char sError[10];
			formatDistance (trackError, pData->navUnits, sError);
			sprintf (sDistance, "%c%s", cSteer, sError);
		}
		break;

	default:
		strcpy (sDistanceMode, "???");
	}

	// Override prompt if active cursor
	if (crsr && (crsrMode == CrsrNavCdiDistance) && !crsrFlash) {
		strcpy (sDistanceMode, "");
	}

	char sTrackMode[10];
	char sTrack[10];
	switch (trkMode) {
	case NAV_TRK_TRACK:
		// Track angle
		strcpy (sTrackMode, "trk");
		formatBearing (pRealTime->trk, sTrack);
		break;

	case NAV_TRK_BEARING:
		// Bearing to waypoint
		strcpy (sTrackMode, "brg");
		formatBearing (brg, sTrack);
		break;

	case NAV_TRK_COURSE:
		// Course to steer
		strcpy (sTrackMode, "cts");
		formatBearing (cts, sTrack);
		break;

	case NAV_TRK_DESIRED_TRACK:
		// Desired track
		strcpy (sTrackMode, "dtk");
		formatBearing (dtk, sTrack);
		break;

	case NAV_TRK_OBS:
		// OBS
		strcpy (sTrackMode, "obs");
		formatBearing (obs, sTrack);
		break;

	case NAV_TRK_TURN:
		// Turn angle
		strcpy (sTrackMode, "trn");
		formatBearing (0, sTrack);
		break;

	default:
		strcpy (sTrackMode, "???");
	}

	// Override prompt if active cursor
	if (crsr && (crsrMode == CrsrNavCdiSteer) && !crsrFlash) {
		strcpy (sTrackMode, "");
	}

	sprintf (pShared->line2, "%-3s %-5s   %-3s %-4s",
		sDistanceMode, sDistance, sTrackMode, sTrack);

	// Line 3, CDI display
	char sCdi[16];
	memset (sCdi, 0, 16);
	if (pData->rteActive.valid) {
		formatCdi (cdiDeflect, sCdi);
	} else {
		strcpy (sCdi, "No actv wpt");
	}

	char sEteMode[10];
	char sEte[10];
	switch (eteMode) {
	case NAV_ETE_ETA:
		strcpy (sEteMode, "eta");
		if (pRte->valid) {
			formatTime (eta, sEte);
		} else {
			strcpy (sEte, "__:__");
		}
		break;

	case NAV_ETE_ETE:
		strcpy (sEteMode, "ete");
		if (pRte->valid) {
			formatTime (ete, sEte);
		} else {
			strcpy (sEte, "__:__");
		}
		break;

	case NAV_ETE_TRK:
		strcpy (sEteMode, "trk");
		formatBearing (pRealTime->trk, sEte);
		break;

	case NAV_ETE_VN:
		strcpy (sEteMode, "vn ");
		formatTime (vn, sEte);
		break;
	}

	// Override ETE prompt if active cursor
	if (crsr && (crsrMode == CrsrNavCdiEte) && !crsrFlash) {
		strcpy (sEteMode, "");
	}

	// Format display line 3
	sprintf (pShared->line3, "%-11s %3s%5s", sCdi, sEteMode, sEte);
}


void Nav::updateNavPositionDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	SGps150RealTimeData *pRealTime = (SGps150RealTimeData *)pData->pRealTime;
	if (!pRealTime) return;

	// Line 1, altitude and (unsupported) msa
	char sAlt[8];
	formatAltitude (pRealTime->userPos.alt, pData->altUnits, sAlt);
	sprintf (pShared->line1, "alt %s", sAlt);

	sprintf (pShared->line1, "alt %6s", sAlt);

	// Set line 2, current position
	char sPos[32];
	formatPosition (pRealTime->userPos, pData->posUnits, sPos);
	strcpy (pShared->line2, sPos);

	// Set line 3, proximity reference
	char sType[8], sName[8], sBearing[8], sDistance[16];
	strcpy (sType, "");
	strcpy (sName, "");
	strcpy (sBearing, "");
	strcpy (sDistance, "");

	switch (proximityMode) {
	case NAV_PROX_APT:
		strcpy (sType, "apt");
		if (pRealTime->nrstApOk) {
			formatAirportId (&pRealTime->nrstAp, sName);

			int iBearing = (int)(pRealTime->nrstApBrgTrue - pData->magVar);
			sprintf (sBearing, "%03d~", iBearing);

			formatDistance (pRealTime->nrstApDistNm, pData->navUnits, sDistance);
		}
		break;

	case NAV_PROX_VOR:
		strcpy (sType, "vor");
		if (pRealTime->nrstVorOk) {
			strcpy (sName, pRealTime->nrstVor.id);

			int iBearing = (int)(pRealTime->nrstVorBrgTrue - pData->magVar);
			sprintf (sBearing, "%03d~", iBearing);

			formatDistance (pRealTime->nrstVorDistNm, pData->navUnits, sDistance);
		}
		break;

	case NAV_PROX_NDB:
		strcpy (sType, "ndb");
		if (pRealTime->nrstNdbOk) {
			strcpy (sName, pRealTime->nrstNdb.id);

			int iBearing = (int)(pRealTime->nrstNdbBrgTrue - pData->magVar);
			sprintf (sBearing, "%03d~", iBearing);

			formatDistance (pRealTime->nrstNdbDistNm, pData->navUnits, sDistance);
		}
		break;

	case NAV_PROX_INT:
		strcpy (sType, "int");
		break;

	case NAV_PROX_USR:
		strcpy (sType, "usr");
		break;

	case NAV_PROX_WPT:
		strcpy (sType, "wpt");
		break;
	}

	// Blank out cursor
	if (!crsrFlash && (crsrMode == CrsrNavPosProx)) {
		strcpy (sType, "");
	}

	sprintf (pShared->line3, "%c%3s:%-4s %4s %3s", CharFrom, sType, sName, sBearing, sDistance);
}


void Nav::updateNavMenu1Display (void)
{
	// Line 1, Trip Plan, Dalt/tas
	char sTripPlan[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu1TripPlan)) {
		strcpy (sTripPlan, "Trip Plan?");
	} else {
		strcpy (sTripPlan, "          ");
	}

	char sDaltTas[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu1DaltTas)) {
		strcpy (sDaltTas, "Dalt/tas?");
	} else {
		strcpy (sDaltTas, "         ");
	}

	sprintf (pShared->line1, "%s %s", sTripPlan, sDaltTas);

	// Line 2, Fuel Plan, Winds
	char sFuelPlan[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu1FuelPlan)) {
		strcpy (sFuelPlan, "Fuel Plan?");
	} else {
		strcpy (sFuelPlan, "          ");
	}

	char sWinds[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu1Winds)) {
		strcpy (sWinds, "Winds?");
	} else {
		strcpy (sWinds, "      ");
	}

	sprintf (pShared->line2, "%s %s", sFuelPlan, sWinds);

	// Line 3, Vnav Plan, Chklist
	char sVnavPlan[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu1VnavPlan)) {
		strcpy (sVnavPlan, "Vnav Plan?");
	} else {
		strcpy (sVnavPlan, "          ");
	}

	char sChklist[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu1Chklist)) {
		strcpy (sChklist, "Chklist?");
	} else {
		strcpy (sChklist, "        ");
	}

	sprintf (pShared->line3, "%s %s", sVnavPlan, sChklist);
}


void Nav::updateNavMenu2Display (void)
{
	char sApprTime[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu2ApprTime)) {
		strcpy (sApprTime, "Appr Time?");
	} else {
		strcpy (sApprTime, "          ");
	}

	char sClock[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu2Clock)) {
		strcpy (sClock, "Clock?");
	} else {
		strcpy (sClock, "      ");
	}

	sprintf (pShared->line1, "%s %s", sApprTime, sClock);

	char sTripTime[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu2TripTime)) {
		strcpy (sTripTime, "Trip Time?");
	} else {
		strcpy (sTripTime, "          ");
	}

	char sGpsPlan[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu2GpsPlan)) {
		strcpy (sGpsPlan, "GPS Plan?");
	} else {
		strcpy (sGpsPlan, "          ");
	}

	sprintf (pShared->line2, "%s %s", sTripTime, sGpsPlan);

	char sScheduler[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu2Scheduler)) {
		strcpy (sScheduler, "Scheduler?");
	} else {
		strcpy (sScheduler, "          ");
	}

	char sSunrise[16];
	if (crsrFlash || (crsrMode != CrsrNavMenu2Sunrise)) {
		strcpy (sSunrise, "Sunrise?");
	} else {
		strcpy (sSunrise, "        ");
	}

	sprintf (pShared->line3, "%s %s", sScheduler, sSunrise);
}

void Nav::updateNavAutoStoreDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, ID
	sprintf (pShared->line1, "Save wpt: %s", idAutoStore);

	// Line 2, user waypoint position
	char sPos[32];
	formatPosition (posAutoStore, pData->posUnits, sPos);
	strcpy (pShared->line2, sPos);

	// Line 3, auto-store in flight plan route
	char sRte[8];
	if ((crsrMode == CrsrNavAutoStoreRte) && !crsrFlash) {
		if (rteAutoStore == -1) {
			strcpy (sRte, "__");
		} else {
			sprintf (sRte, "%02d", rteAutoStore);
		}
	} else {
		strcpy (sRte, "");
	}

	char sOk[8];
	if ((crsrMode == CrsrNavAutoStoreOk) && !crsrFlash) {
		strcpy (sOk, "ok?");
	} else {
		strcpy (sOk, "");
	}
	sprintf (pShared->line3, " Store in rte:%2s %3s", sRte, sOk);
}

void Nav::updateNavTripPlanDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	strcpy (pShared->line1, "     Trip Plan     ");
	strcpy (pShared->line2, "  Not implemented  ");
	strcpy (pShared->line3, "");
}

void Nav::updateNavDaltTasDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, indicated alt, calibrated airspeed
	char sIalt[16];
	float ialt = fsimUserMSL ();
	formatAltitude (ialt, pData->altUnits, sIalt);

	char sCas[16];
	float cas = fsimUserCAS ();
	formatSpeed (cas, pData->navUnits, sCas);

	sprintf (pShared->line1, "ialt:%s cas:%s", sIalt, sCas);

	// Line 2, pressure, total air temperature
	char sPress[16];
	float press = fsimUserPrimaryKollsmanSetting ();
	sprintf (sPress, "%04.2f", press);

	char sTat[16];
	float tat = fsimUserTAT ();
	formatTemperature (tat, pData->tempUnits, sTat);

	sprintf (pShared->line2, "pres:%s  tat:%s", sPress, sTat);

	// Line 3, density alt, true airspeed
	char sDalt[16];
	float dalt = fsimUserDensityAlt();
	formatAltitude (dalt, pData->altUnits, sDalt);

	char sTas[16];
	float tas = fsimUserTAS ();
	formatSpeed (tas, pData->navUnits, sTas);

	sprintf (pShared->line3, "dalt:%s tas%s", sDalt, sTas);
}


void Nav::updateNavFuelPlanDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	strcpy (pShared->line1, "Fuel Plan");
	strcpy (pShared->line2, "Not implemented");
	strcpy (pShared->line3, "");
}


void Nav::updateNavWindsDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, user heading and TAS
	char sHdg[16];
	float hdg = fsimUserTrueHeading ();
	formatBearing (hdg, sHdg);

	char sTas[16];
	float tas = fsimUserTAS ();
	formatSpeed (tas, pData->navUnits, sTas);

	sprintf (pShared->line1, "hdg: %s tas: %s", sHdg, sTas);

	// Line 2, wind direction and speed
	char sWindHdg[16], sWindSpeed[16];
	float windHdg, windSpeed;

	bool onGround = fsimUserAllWheelsOnGround ();
	if (onGround) {
		// Aircraft is on ground wind speed/direction would not be able to
		//   be calculated by the GPS so null the values
		windHdg = 0;
		windSpeed = 0;
	} else {
		// Get actual wind heading and speed
		windHdg = fsimUserWindTrueHeading ();
		windSpeed = fsimUserWindSpeed ();
	}

	formatBearing (windHdg, sWindHdg);
	formatSpeed (FpsToKt(windSpeed), pData->navUnits, sWindSpeed);

	sprintf (pShared->line2, "wind %s at %s", sWindHdg, sWindSpeed);

	// Line 3, head/tailwind
	float headWind = fsimUserHeadWindSpeed ();
	float tailWind = fsimUserTailWindSpeed ();
	float diffSpeed;

	char sDir[16];
	if (headWind > tailWind) {
		strcpy (sDir, "tail");
		diffSpeed = headWind;
	} else {
		strcpy (sDir, "head");
		diffSpeed = tailWind;
	}

	char sDiffSpeed[16];
	formatSpeed (FpsToKt (diffSpeed), pData->navUnits, sDiffSpeed);

	sprintf (pShared->line3, "%s wind is %s", sDir, sDiffSpeed);
}

void Nav::updateNavVnavPlanDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	strcpy (pShared->line1, "Vnav");
	strcpy (pShared->line2, "Not implemented");
	strcpy (pShared->line3, "");
}

void Nav::updateNavChklistMenuDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, title
	strcpy (pShared->line1, "Select check list");

	// Line 2, first checklist title
	int i = iChecklist - iChecklistSelect;
	char sTitle[20];
	if ((i >= pData->nChecklists) || ((i == iChecklist) && !crsrFlash)) {
		strcpy (sTitle, "");
	} else {
		strcpy (sTitle, pData->checklist[i].name);
	}
	sprintf (pShared->line2, "%2d:%s", i+1, sTitle);
	i++;

	// Line 3, second checklist title
	if ((i >= pData->nChecklists) || ((i == iChecklist) && !crsrFlash)) {
		strcpy (sTitle, "");
	} else {
		strcpy (sTitle, pData->checklist[i].name);
	}
	sprintf (pShared->line3, "%2d:%s", i+1, sTitle);
}

void Nav::updateNavChklistDetailDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	SChecklist *c = &pData->checklist[iChecklist];

	// Line 1, Checklist name
	strcpy (pShared->line1, c->name);

	// Line 2, first item
	int i = iChecklistItem - iChecklistItemSelect;
	char sItem[20];
	if ((i >= c->nItems) || ((i == iChecklistItem) && !crsrFlash)) {
		strcpy (sItem, "");
	} else {
		strcpy (sItem, c->item[i]);
	}

	char cDone = c->done[i] ? '@' : ' ';

	sprintf (pShared->line2, "%c%2d:%s", cDone, i+1, sItem);
	i++;

	// Line 3, second item
	if ((i >= c->nItems) || ((i == iChecklistItem) && !crsrFlash)) {
		strcpy (sItem, "");
	} else {
		strcpy (sItem, c->item[i]);
	}

	cDone = c->done[i] ? '@' : ' ';

	sprintf (pShared->line3, "%c%2d:%s", cDone, i+1, sItem);
}

void Nav::updateNavApprTimeDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	//  Update approach timer hour, minute, second
	int secs = (int)pData->apprTimer;
	apprTimeHour = secs / 3600;
	apprTimeMin = (secs - apprTimeHour*60) / 60;
	apprTimeSec = secs % 60;
	
	// Line 1 : Count down/up
	char sDir[8];
	char sFromTo[8];
	if (pData->apprTimerCountDown) {
		strcpy (sDir, "down");
		strcpy (sFromTo, "from");
	} else {
		strcpy (sDir, " up ");
		strcpy (sFromTo, " to ");
	}

	// Blank direction if current crsr selection
	if (!crsrFlash && (crsrMode == CrsrApprTimeDir)) {
		strcpy (sDir, "    ");
	}
	sprintf (pShared->line1, "Count %s timer", sDir);

	// Line 2 : setting
	char sHour[8];
	char sMinute[8];
	char sSecond[8];

	sprintf (sHour, "%1d", apprTimeHour);
	sprintf (sMinute, "%02d", apprTimeMin);
	sprintf (sSecond, "%02d", apprTimeSec);

	if (!crsrFlash) {
		switch (crsrMode) {
		case CrsrApprTimeHours:
			sHour[0] = ' ';
			break;

		case CrsrApprTimeMinTens:
			sMinute[0] = ' ';
			break;

		case CrsrApprTimeMinOnes:
			sMinute[1] = ' ';
			break;

		case CrsrApprTimeSecTens:
			sSecond[0] = ' ';
			break;

		case CrsrApprTimeSecOnes:
			sSecond[1] = ' ';
			break;
		}
	}

	sprintf (pShared->line2, " %s %s:%s:%s", sFromTo, sHour, sMinute, sSecond);

	// Line 3 : Menu
	char sStart[8];
	if (!crsrFlash && (crsrMode == CrsrApprTimeStart)) {
		strcpy (sStart, "      ");
	} else {
		strcpy (sStart, "Start?");
	}

	char sStop[8];
	if (!crsrFlash && (crsrMode == CrsrApprTimeStop)) {
		strcpy (sStop, "     ");
	} else {
		strcpy (sStop, "Stop?");
	}

	char sReset[8];
	if (!crsrFlash && (crsrMode == CrsrApprTimeReset)) {
		strcpy (sReset, "      ");
	} else {
		strcpy (sReset, "Reset?");
	}

	sprintf (pShared->line3, "%s %s %s", sStart, sStop, sReset);
}


void Nav::updateNavClockDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, time mode (UTC or lcl)
	char sTimeMode[16];
	if (!crsrFlash && (crsrMode == CrsrClockTimeMode)) {
		// Blank time mode
		strcpy (sTimeMode, "   ");
	} else {
		if (pData->useUTC) {
			strcpy (sTimeMode, "UTC");
		} else {
			strcpy (sTimeMode, "lcl");
		}
	}
	sprintf (pShared->line1, "Select local (%s)", sTimeMode);

	// Line 2, UTC time
	char sDateUTC[16], sTimeUTC[16];
	formatDate (pData->utcDate, sDateUTC);
	sprintf (sTimeUTC, "%02d:%02d:%02d",
		pData->utcTime.hour, pData->utcTime.minute, pData->utcTime.second);
	sprintf (pShared->line2, "%s %s  U", sDateUTC, sTimeUTC);

	// Calculate local date/time
	SFsimDate lclDate;
	SFsimTime lclTime;
	calcLclDateTime (pData->utcDate, pData->utcTime, pData->lclOffset, lclDate, lclTime);

	// Line 3, Local time
	char sDateLcl[16], sTimeLcl[16];
	formatDate (lclDate, sDateLcl);
	sprintf (sTimeLcl, "%02d:%02d", lclTime.hour, lclTime.minute);
	if (!crsrFlash) {
		switch (crsrMode) {
		case CrsrClockLclHour:
			sTimeLcl[0] = ' ';
			sTimeLcl[1] = ' ';
			break;

		case CrsrClockLclMin:
			sTimeLcl[3] = ' ';
			sTimeLcl[4] = ' ';
			break;
		}
	}
	sprintf (pShared->line3, "%s %s   lcl", sDateLcl, sTimeLcl);
}

void Nav::updateNavTripTimeDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, Current time
	char sCurrent[16];
	if (pData->useUTC) {
		sprintf (sCurrent, "%02d:%02d", pData->utcTime.hour, pData->utcTime.minute);
	} else {
		SFsimDate lclDate;
		SFsimTime lclTime;
		calcLclDateTime (pData->utcDate, pData->utcTime, pData->lclOffset,
			lclDate, lclTime);
		sprintf (sCurrent, "%02d:%02d", lclTime.hour, lclTime.minute);
	}
	sprintf (pShared->line1, "Time %s", sCurrent);

	// Line 2, Departure time
	char sDeparture[16];
	if (pData->useUTC) {
		sprintf (sDeparture, "%02d:%02d", pData->utcDepart.hour, pData->utcDepart.minute);
	} else {
		// Calculate local time
		SFsimDate lclDate;
		SFsimTime lclTime;
		calcLclDateTime (pData->utcDate, pData->utcTime, pData->lclOffset, lclDate, lclTime);
		sprintf (sDeparture, "%02d:%02d", lclTime.hour, lclTime.minute);
	}
	sprintf (pShared->line2, "Dep  %s", sDeparture);

	// Line 3, trip time and reset
	char sTrip[16];
	SFsimTime trip;
	int mins = (int)(pData->tripTimer / 60);
	trip.hour = mins / 60;
	trip.minute = mins % 60;
	sprintf (sTrip, "%02d:%02d", trip.hour, trip.minute);

	char sReset[16];
	if (crsrFlash) {
		strcpy (sReset, "Reset?");
	} else {
		strcpy (sReset, "");
	}
	sprintf (pShared->line3, "Trip %s    %s", sTrip, sReset);
}

void Nav::updateNavGpsPlanDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, current date
	char sDate[16];
	if (pData->useUTC) {
		formatDate (pData->utcDate, sDate);
	} else {
		SFsimDate lclDate;
		SFsimTime lclTime;
		calcLclDateTime (pData->utcDate, pData->utcTime, pData->lclOffset,
			lclDate, lclTime);
		formatDate (lclDate, sDate);
	}
	sprintf (pShared->line1, "wpt: ALL   %s", sDate);

	// Line 2, constant "fix 3D"
	strcpy (pShared->line2, "fix 3D");

	// Line 3, constant "24 hour coverage"
	strcpy (pShared->line3, "24 hour coverage");
}


void Nav::updateNavSchedulerDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, title
	strcpy (pShared->line1, "Scheduled messages");

	// Line 2, first selected schedule item
	int i = iScheduler - iSchedulerSelect;
	char sName[16];
	if (pData->scheduledMsg[i].active) {
		if (crsrFlash || (i != iScheduler)) {
			strcpy (sName, pData->scheduledMsg[i].name);
		} else {
			strcpy (sName, "            ");
		}
	} else {
		strcpy (sName, "____________");
	}

	char sTime[16];
	if (pData->scheduledMsg[i].active) {
		sprintf (sTime, "%02d:%02d",
			pData->scheduledMsg[i].time.hour, pData->scheduledMsg[i].time.minute);
	} else {
		strcpy (sTime, "__:__");
	}
	sprintf (pShared->line2, "%1d:%12s %5s", i+1, sName, sTime);
	i++;

	// Line 2, first selected schedule item
	if (pData->scheduledMsg[i].active) {
		// Item is active
		if (crsrFlash || (i != iScheduler)) {
			strcpy (sName, pData->scheduledMsg[i].name);
		} else {
			strcpy (sName, "            ");
		}
	} else {
		strcpy (sName, "____________");
	}

	if (pData->scheduledMsg[i].active) {
		sprintf (sTime, "%02d:%02d",
			pData->scheduledMsg[i].time.hour, pData->scheduledMsg[i].time.minute);
	} else {
		strcpy (sTime, "__:__");
	}
	sprintf (pShared->line3, "%1d:%12s %s", i+1, sName, sTime);
}


void Nav::updateNavSunriseDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	strcpy (pShared->line1, "Sunrise/sunset");

	// Line 2, waypoint and date
	char sType[8];
	char sName[8];
	if (crsr && (crsrMode == CrsrSunriseWpt)) {
		strcpy (sType, "wpt");
		strcpy (sName, textValue);
		if (!crsrFlash) {
			sName[iText] = ' ';
		}
	} else {
		// CRSR not selected, display activated waypoint data
		switch (wptSunrise.type) {
		case WptAirport:
			strcpy (sType, "apt");
			break;

		case WptVor:
			strcpy (sType, "vor");
			break;

		case WptNdb:
			strcpy (sType, "ndb");
			break;

		case WptFix:
			strcpy (sType, "int");
			break;

		case WptUser:
			strcpy (sType, "usr");
			break;
		}
		strcpy (sName, wptSunrise.id);
	}

	// Test case
	// June 25, 1990:	25, 6, 1990
	// Wayne, NJ:      40.9, -74.3
	// Office zenith:  90 50' cos(zenith) = -0.01454
	//	date.year = 1990;
	//	date.month = 6;
	//	date.day = 25;
	//	pos.lat = 40.9;
	//	pos.lon = -74.3;

	// Format date
	char sDay[8], sMonth[8], sYear[8];

	if (!crsrFlash && (crsrMode == CrsrSunriseDay)) {
		strcpy (sDay, "  ");
	} else {
		sprintf (sDay, "%02d", dateSunrise.day);
	}

	if (!crsrFlash && (crsrMode == CrsrSunriseMonth)) {
		strcpy (sMonth, "   ");
	} else {
		formatMonth (dateSunrise.month, sMonth);
	}

	if (!crsrFlash && (crsrMode == CrsrSunriseYear)) {
		strcpy (sYear, "  ");
	} else {
		sprintf (sYear, "%02d", dateSunrise.year % 100);
	}

	sprintf (pShared->line2, "%s: %s  %s-%s-%s", sType, sName, sDay, sMonth, sYear);

	SFsimPosition pos = wptSunrise.pos;

	// Convert position arcseconds to lat/lon degrees
	pos.lat /= 3600.0f;
	pos.lon /= 3600.0f;
	if (pos.lon > 180) pos.lon -= 360;

	// Line 3, rise/set times
	SFsimTime utcRise, utcSet;
	bool neverRises, neverSets;
	calcSunriseSunset (pos, dateSunrise, 90.833f, utcRise, utcSet, neverRises, neverSets);

	// Adjust to local time if necessary
	SFsimTime rise, set;
	SFsimDate date = dateSunrise;
	SFsimDate copyDate = date;
	if (pData->useUTC) {
		rise = utcRise;
		set = utcSet;
	} else {
		// Calculate local times
		calcLclDateTime (copyDate, utcRise, pData->lclOffset, date, rise);
		calcLclDateTime (copyDate, utcSet, pData->lclOffset, date, set);
	}

	char sRise[16];
	if (neverRises) {
		strcpy (sRise, "--:--");
	} else {
		sprintf (sRise, "%02d:%02d", rise.hour, rise.minute);
	}

	char sSet[16];
	if (neverSets) {
		strcpy (sSet, "--:--");
	} else {
		sprintf (sSet, "%02d:%02d", set.hour, set.minute);
	}

	sprintf (pShared->line3, "Rise %s Set %s", sRise, sSet);
}



void Nav::updateNavDisplay ()
{
	switch (page) {
		case NavCdi:
			updateNavCdiDisplay ();
			break;

		case NavPosition:
			updateNavPositionDisplay ();
			break;

		case NavMenu1:
			updateNavMenu1Display ();
			break;

		case NavMenu2:
			updateNavMenu2Display ();
			break;

		case NavAutoStore:
			updateNavAutoStoreDisplay ();
			break;

		case NavTripPlan:
			updateNavTripPlanDisplay ();
			break;

		case NavDaltTas:
			updateNavDaltTasDisplay ();
			break;

		case NavFuelPlan:
			updateNavFuelPlanDisplay ();
			break;

		case NavWinds:
			updateNavWindsDisplay ();
			break;

		case NavVnavPlan:
			updateNavVnavPlanDisplay ();
			break;

		case NavChklistMenu:
			updateNavChklistMenuDisplay ();
			break;

		case NavChklistDetail:
			updateNavChklistDetailDisplay ();
			break;

		case NavApprTime:
			updateNavApprTimeDisplay ();
			break;

		case NavClock:
			updateNavClockDisplay ();
			break;

		case NavTripTime:
			updateNavTripTimeDisplay ();
			break;

		case NavGpsPlan:
			updateNavGpsPlanDisplay ();
			break;

		case NavScheduler:
			updateNavSchedulerDisplay ();
			break;

		case NavSunrise:
			updateNavSunriseDisplay ();
			break;
	}
}

static ENavPage incNavPage (ENavPage page)
{
	ENavPage rc = NavCdi;

	switch (page) {
		case NavCdi:
			rc = NavPosition;
			break;

		case NavPosition:
			rc = NavMenu1;
			break;

		case NavMenu1:
			rc = NavMenu2;
			break;

		case NavMenu2:
			rc = NavCdi;
			break;
	}

	return rc;
}


static ENavPage decNavPage (ENavPage page)
{
	ENavPage rc = NavCdi;

	switch (page) {
		case NavCdi:
			rc = NavMenu2;
			break;

		case NavPosition:
			rc = NavCdi;
			break;

		case NavMenu1:
			rc = NavPosition;
			break;

		case NavMenu2:
			rc = NavMenu1;
			break;
	}

	return rc;
}


void incNavCdiCrsrMode (int &mode)
{
	switch (mode) {
//	case CrsrNavCdiIdent:
//		mode = CrsrNavCdiGs;
//		break;

//	case CrsrNavCdiGs:
//		mode = CrsrNavCdiDistance;
//		break;

	case CrsrNavCdiDistance:
		mode = CrsrNavCdiSteer;
		break;

	case CrsrNavCdiSteer:
		mode = CrsrNavCdiEte;
		break;

	case CrsrNavCdiEte:
		mode = CrsrNavCdiDistance;
		break;
	}
}


void decNavCdiCrsrMode (int &mode)
{
	switch (mode) {
//	case CrsrNavCdiIdent:
//		mode = CrsrNavCdiEte;
//		break;

//	case CrsrNavCdiGs:
//		mode = CrsrNavCdiIdent;
//		break;

	case CrsrNavCdiDistance:
		mode = CrsrNavCdiEte;
		break;

	case CrsrNavCdiSteer:
		mode = CrsrNavCdiDistance;
		break;

	case CrsrNavCdiEte:
		mode = CrsrNavCdiSteer;
		break;
	}
}


void incNavDistanceMode (ENavDistanceMode &mode)
{
	switch (mode) {
	case NAV_DIS_DISTANCE:
		mode = NAV_DIS_STEER;
		break;

	case NAV_DIS_STEER:
		mode = NAV_DIS_DISTANCE;
		break;
	}
}

void decNavDistanceMode (ENavDistanceMode &mode)
{
	switch (mode) {
	case NAV_DIS_DISTANCE:
		mode = NAV_DIS_STEER;
		break;

	case NAV_DIS_STEER:
		mode = NAV_DIS_DISTANCE;
		break;
	}
}

void incNavTrackMode (ENavTrackMode &mode)
{
	switch (mode) {
	case NAV_TRK_TRACK:
		mode = NAV_TRK_BEARING;
		break;

	case NAV_TRK_BEARING:
		mode = NAV_TRK_COURSE;
		break;

	case NAV_TRK_COURSE:
		mode = NAV_TRK_DESIRED_TRACK;
		break;

	case NAV_TRK_DESIRED_TRACK:
		mode = NAV_TRK_OBS;
		break;

	case NAV_TRK_OBS:
		mode = NAV_TRK_TURN;
		break;

	case NAV_TRK_TURN:
		mode = NAV_TRK_TRACK;
		break;
	}
}

void decNavTrackMode (ENavTrackMode &mode)
{
	switch (mode) {
	case NAV_TRK_TRACK:
		mode = NAV_TRK_TURN;
		break;

	case NAV_TRK_BEARING:
		mode = NAV_TRK_TRACK;
		break;

	case NAV_TRK_COURSE:
		mode = NAV_TRK_BEARING;
		break;

	case NAV_TRK_DESIRED_TRACK:
		mode = NAV_TRK_COURSE;
		break;

	case NAV_TRK_OBS:
		mode = NAV_TRK_DESIRED_TRACK;
		break;

	case NAV_TRK_TURN:
		mode = NAV_TRK_OBS;
		break;
	}
}

void incNavEteMode (ENavEteMode &mode)
{
	switch (mode) {
	case NAV_ETE_ETA:
		mode = NAV_ETE_ETE;
		break;

	case NAV_ETE_ETE:
		mode = NAV_ETE_TRK;
		break;

	case NAV_ETE_TRK:
		mode = NAV_ETE_VN;
		break;

	case NAV_ETE_VN:
		mode = NAV_ETE_ETA;
		break;
	}
}

void decNavEteMode (ENavEteMode &mode)
{
	switch (mode) {
	case NAV_ETE_ETA:
		mode = NAV_ETE_VN;
		break;

	case NAV_ETE_ETE:
		mode = NAV_ETE_ETA;
		break;

	case NAV_ETE_TRK:
		mode = NAV_ETE_ETE;
		break;

	case NAV_ETE_VN:
		mode = NAV_ETE_TRK;
		break;
	}
}


void incNavClockCrsrMode (int &mode)
{
	switch (mode) {
	case CrsrClockTimeMode:
		// No scrolling from this mode
		break;

	case CrsrClockLclHour:
		mode = CrsrClockLclMin;
		break;

	case CrsrClockLclMin:
		mode = CrsrClockLclHour;
		break;
	}
}


void decNavClockCrsrMode (int &mode)
{
	switch (mode) {
	case CrsrClockTimeMode:
		// No scrolling from this mode
		break;

	case CrsrClockLclHour:
		mode = CrsrClockLclMin;
		break;

	case CrsrClockLclMin:
		mode = CrsrClockLclHour;
		break;
	}
}


void incNavApprTimeCrsrMode (int &mode)
{
	switch (mode) {
	case CrsrApprTimeHours:
		mode = CrsrApprTimeMinTens;
		break;

	case CrsrApprTimeMinTens:
		mode = CrsrApprTimeMinOnes;
		break;

	case CrsrApprTimeMinOnes:
		mode = CrsrApprTimeSecTens;
		break;

	case CrsrApprTimeSecTens:
		mode = CrsrApprTimeSecOnes;
		break;

	case CrsrApprTimeSecOnes:
		mode = CrsrApprTimeHours;
		break;

	case CrsrApprTimeStart:
		mode = CrsrApprTimeStop;
		break;

	case CrsrApprTimeStop:
		mode = CrsrApprTimeReset;
		break;

	case CrsrApprTimeReset:
		mode = CrsrApprTimeStart;
		break;
	}
}


void decNavApprTimeCrsrMode (int &mode)
{
	switch (mode) {
	case CrsrApprTimeHours:
		mode = CrsrApprTimeSecOnes;
		break;

	case CrsrApprTimeMinTens:
		mode = CrsrApprTimeHours;
		break;

	case CrsrApprTimeMinOnes:
		mode = CrsrApprTimeMinTens;
		break;

	case CrsrApprTimeSecTens:
		mode = CrsrApprTimeMinOnes;
		break;

	case CrsrApprTimeSecOnes:
		mode = CrsrApprTimeSecTens;
		break;

	case CrsrApprTimeStart:
		mode = CrsrApprTimeReset;
		break;

	case CrsrApprTimeStop:
		mode = CrsrApprTimeStart;
		break;

	case CrsrApprTimeReset:
		mode = CrsrApprTimeStop;
		break;
	}
}


void incNavAutoStoreCrsrMode (int &mode)
{
	switch (mode) {
	case CrsrNavAutoStoreId:
		mode = CrsrNavAutoStoreRte;
		break;

	case CrsrNavAutoStoreRte:
		mode = CrsrNavAutoStoreOk;
		break;

	case CrsrNavAutoStoreOk:
		mode = CrsrNavAutoStoreId;
		break;
	}
}


void decNavAutoStoreCrsrMode (int &mode)
{
	switch (mode) {
	case CrsrNavAutoStoreId:
		mode = CrsrNavAutoStoreOk;
		break;

	case CrsrNavAutoStoreRte:
		mode = CrsrNavAutoStoreId;
		break;

	case CrsrNavAutoStoreOk:
		mode = CrsrNavAutoStoreRte;
		break;
	}
}


void incNavMenu1CrsrMode (int &mode)
{
	switch (mode) {
	case CrsrNavMenu1TripPlan:
		mode = CrsrNavMenu1DaltTas;
		break;

	case CrsrNavMenu1DaltTas:
		mode = CrsrNavMenu1FuelPlan;
		break;

	case CrsrNavMenu1FuelPlan:
		mode = CrsrNavMenu1Winds;
		break;

	case CrsrNavMenu1Winds:
		mode = CrsrNavMenu1VnavPlan;
		break;

	case CrsrNavMenu1VnavPlan:
		mode = CrsrNavMenu1Chklist;
		break;

	case CrsrNavMenu1Chklist:
		mode = CrsrNavMenu1TripPlan;
		break;
	}
}


void decNavMenu1CrsrMode (int &mode)
{
	switch (mode) {
	case CrsrNavMenu1TripPlan:
		mode = CrsrNavMenu1Chklist;
		break;

	case CrsrNavMenu1DaltTas:
		mode = CrsrNavMenu1TripPlan;
		break;

	case CrsrNavMenu1FuelPlan:
		mode = CrsrNavMenu1DaltTas;
		break;

	case CrsrNavMenu1Winds:
		mode = CrsrNavMenu1FuelPlan;
		break;

	case CrsrNavMenu1VnavPlan:
		mode = CrsrNavMenu1Winds;
		break;

	case CrsrNavMenu1Chklist:
		mode = CrsrNavMenu1VnavPlan;
		break;
	}
}


void incNavMenu2CrsrMode (int &mode)
{
	switch (mode) {
	case CrsrNavMenu2ApprTime:
		mode = CrsrNavMenu2Clock;
		break;

	case CrsrNavMenu2Clock:
		mode = CrsrNavMenu2TripTime;
		break;

	case CrsrNavMenu2TripTime:
		mode = CrsrNavMenu2GpsPlan;
		break;

	case CrsrNavMenu2GpsPlan:
		mode = CrsrNavMenu2Scheduler;
		break;

	case CrsrNavMenu2Scheduler:
		mode = CrsrNavMenu2Sunrise;
		break;

	case CrsrNavMenu2Sunrise:
		mode = CrsrNavMenu2ApprTime;
		break;
	}
}


void decNavMenu2CrsrMode (int &mode)
{
	switch (mode) {
	case CrsrNavMenu2ApprTime:
		mode = CrsrNavMenu2Sunrise;
		break;

	case CrsrNavMenu2Clock:
		mode = CrsrNavMenu2ApprTime;
		break;

	case CrsrNavMenu2TripTime:
		mode = CrsrNavMenu2Clock;
		break;

	case CrsrNavMenu2GpsPlan:
		mode = CrsrNavMenu2TripTime;
		break;

	case CrsrNavMenu2Scheduler:
		mode = CrsrNavMenu2GpsPlan;
		break;

	case CrsrNavMenu2Sunrise:
		mode = CrsrNavMenu2Scheduler;
		break;
	}
}


void incNavProximityMode (ENavProximity &proximityMode)
{
	switch (proximityMode) {
	case NAV_PROX_APT:
		proximityMode = NAV_PROX_VOR;
		break;

	case NAV_PROX_VOR:
		proximityMode = NAV_PROX_NDB;
		break;

	case NAV_PROX_NDB:
		proximityMode = NAV_PROX_APT;   // TEMP : Should be INT
		break;

	case NAV_PROX_INT:
		proximityMode = NAV_PROX_WPT;
		break;

	case NAV_PROX_WPT:
		proximityMode = NAV_PROX_USR;
		break;

	case NAV_PROX_USR:
		proximityMode = NAV_PROX_APT;
		break;
	}
} 


void decNavProximityMode (ENavProximity &proximityMode)
{
	switch (proximityMode) {
	case NAV_PROX_APT:
		proximityMode = NAV_PROX_NDB;   // TEMP: Should be USR
		break;

	case NAV_PROX_VOR:
		proximityMode = NAV_PROX_APT;
		break;

	case NAV_PROX_NDB:
		proximityMode = NAV_PROX_VOR;
		break;

	case NAV_PROX_INT:
		proximityMode = NAV_PROX_NDB;
		break;

	case NAV_PROX_WPT:
		proximityMode = NAV_PROX_INT;
		break;

	case NAV_PROX_USR:
		proximityMode = NAV_PROX_WPT;
		break;
	}
}



Nav::Nav (void *p) :
	FsmState (p),
	crsr(false),
	page(NavPosition),
	disMode(NAV_DIS_DISTANCE),
	trkMode(NAV_TRK_TRACK),
	eteMode(NAV_ETE_ETA),
	proximityMode(NAV_PROX_APT),
	iChecklist (0), iChecklistSelect(0),
	iChecklistItem (0), iChecklistItemSelect(0),
	iScheduler(0), iSchedulerSelect(0)
{
}

void Nav::Activate (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	SGps150RealTimeData *pRealTime = pData->pRealTime;

	// Set indicator light in shared data
	pShared->liteNav = true;

	// Check whether activation is due to waypoint confirmation
	if (wpSunrisePending) {
		wpSunrisePending = false;

		if (pData->wpActive.confirmed) {
			// Transfer waypoint info to sunrise waypoint
			wptSunrise.type = pData->wpActive.type;
			strcpy (wptSunrise.id, pData->wpActive.id);
			wptSunrise.pos = pData->wpActive.pos;
			wptSunrise.confirmed = true;
		}
	} else {
		// Initialize sunrise/sunset waypoint
		if (pRealTime->nrstApOk) {
			// Copy nearest airport data into sunrise/sunset waypoint
			wptSunrise.type = WptAirport;
			formatAirportId (&pRealTime->nrstAp, wptSunrise.id);
			wptSunrise.pos = pRealTime->nrstAp.pos;
			wptSunrise.confirmed = true;
 		} else if (pRealTime->nrstVorOk) {
			// Copy nearest VOR data into sunrise/sunset waypoint
			wptSunrise.type = WptVor;
			strcpy (wptSunrise.id, pRealTime->nrstVor.id);
			wptSunrise.pos = pRealTime->nrstVor.pos;
			wptSunrise.confirmed = true;
		} else if (pRealTime->nrstNdbOk) {
			// Copy nearest NDB data into sunrise/sunset waypoint
			wptSunrise.type = WptNdb;
			strcpy (wptSunrise.id, pRealTime->nrstNdb.id);
			wptSunrise.pos = pRealTime->nrstNdb.pos;
			wptSunrise.confirmed = true;
		} else  {
			// No nearest waypoint
			wptSunrise.type = WptUser;
			strcpy (wptSunrise.id, "____");
			wptSunrise.pos = pRealTime->userPos;
			wptSunrise.confirmed = true;
		}
		dateSunrise = fsimCurrentDate ();
	}

	// Initialize sunrise/sunset date/time

	elapsed = 0;
	updateNavDisplay ();
}


void Nav::Deactivate (void)
{
	// Set indicator light in shared data
	pShared->liteNav = false;
}


bool Nav::HandleTrigger (FsmTrigger t)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	bool rc = false;

	switch (t) {
		case TriggerENT:
			// Auto-save user waypoint if on Position page
			switch (page) {
			case NavPosition:
				// Check that user waypoint database is not full
				if (pData->nUserWaypoints < 100) {
					// Initialize auto-save parameters
					sprintf (idAutoStore, "+%03d", pData->nUserWaypoints);
					posAutoStore = pRealTime->userPos;
					rteAutoStore = -1;
					page = NavAutoStore;
				}
				break;

			case NavAutoStore:
				if (!crsr || (crsr && (crsrMode == CrsrNavAutoStoreOk))) {
					// Add auto-store waypoint to user waypoint database
					SUserWaypoint *pUser = &pData->userWpDatabase[pData->nUserWaypoints];
					strcpy (pUser->name, idAutoStore);
					pUser->pos = posAutoStore;
					pData->nUserWaypoints++;
					page = NavPosition;
					break;
				}
				break;

			case NavMenu1:
				switch (crsrMode) {
				case CrsrNavMenu1TripPlan:
					page = NavTripPlan;
					break;

				case CrsrNavMenu1DaltTas:
					page = NavDaltTas;
					break;

				case CrsrNavMenu1FuelPlan:
					page = NavFuelPlan;
					break;

				case CrsrNavMenu1Winds:
					page = NavWinds;
					break;

				case CrsrNavMenu1VnavPlan:
					page = NavVnavPlan;
					break;

				case CrsrNavMenu1Chklist:
					page = NavChklistMenu;
					break;
				}
				break;

			case NavMenu2:
				switch (crsrMode) {
				case CrsrNavMenu2ApprTime:
					page = NavApprTime;
					crsrMode = CrsrApprTimeDir;
					break;

				case CrsrNavMenu2Clock:
					page = NavClock;
					crsrMode = CrsrClockTimeMode;
					break;

				case CrsrNavMenu2TripTime:
					page = NavTripTime;
					break;

				case CrsrNavMenu2GpsPlan:
					page = NavGpsPlan;
					break;

				case CrsrNavMenu2Scheduler:
					page = NavScheduler;
					crsrMode = CrsrNavSchedulerName;
					break;

				case CrsrNavMenu2Sunrise:
					// Switch to Sunrise/Sunset page
					page = NavSunrise;

					// Prepare for Sunrise waypoint ID edit mode
					crsrMode = CrsrSunriseWpt;
					iText = 0;
					iTextBound = 4;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptSunrise.id);
					break;
				}
				break;

			case NavChklistMenu:
				page = NavChklistDetail;
				iChecklistItem = 0;
				iChecklistItemSelect = 0;
				break;

			case NavChklistDetail:
				// Toggle done flag
				if (iChecklist < pData->nChecklists) {
					SChecklist *c = &pData->checklist[iChecklist];
					if (iChecklistItem < c->nItems) {
						c->done[iChecklistItem] = !c->done[iChecklistItem];
					}
				}

				// Increment checklist item line select and/or index
				if (iChecklistItem < pData->checklist[iChecklist].nItems-1) {
					// OK to advance to the next checklist
					iChecklistItem++;
					if (iChecklistItemSelect == 0) {
						// Current selecting first line, advance to second line
						iChecklistItemSelect = 1;
					}
				}
				break;

			case NavClock:
				switch (crsrMode) {
				case CrsrClockTimeMode:
					// Advance to local time adjust mode
					crsrMode = CrsrClockLclHour;
					break;

				case CrsrClockLclHour:
				case CrsrClockLclMin:
					page = NavMenu2;
					crsrMode = CrsrNavMenu2Clock;
					// Return to menu
					break;
				}

			case NavApprTime:
				switch (crsrMode) {
				case CrsrApprTimeDir:
					// Advance to hour
					crsrMode = CrsrApprTimeHours;
					break;

				case CrsrApprTimeHours:
				case CrsrApprTimeMinTens:
				case CrsrApprTimeMinOnes:
				case CrsrApprTimeSecTens:
				case CrsrApprTimeSecOnes:
					// Advance to menu
					crsrMode = CrsrApprTimeStart;
					break;

				case CrsrApprTimeStart:
					// Store updated timer value to preset
					pData->apprTimerPreset = pData->apprTimer;

					// Initialize timer for counting
					if (!pData->apprTimerCountDown) {
						pData->apprTimer = 0;
					}
					pData->apprTimerActive = true;
					break;

				case CrsrApprTimeStop:
					// Simply disable timer
					pData->apprTimerActive = false;
					break;

				case CrsrApprTimeReset:
					// Initialize timer for counting
					if (pData->apprTimerCountDown) {
						pData->apprTimer = pData->apprTimerPreset;
					} else {
						pData->apprTimer = 0;
					}
					pData->apprTimerActive = true;
					break;
				}
				break;

			case NavTripTime:
				// Reset trip timer
				pData->tripTimer = 0;
				pData->departed = false;
				memset (&pData->utcDepart, 0, sizeof(SFsimTime));
				break;

			case NavSunrise:
				switch (crsrMode) {
				case CrsrSunriseWpt:
					// Advance cursor to year
					crsrMode = CrsrSunriseDay;

					// Confirm waypoint entry if it exists
					if (searchWaypointId (pData, textValue, &pData->wpActive)) {
						wpSunrisePending = true;
						pData->replyTrigger = TriggerNAV;
						pData->fsm.HandleTrigger (TriggerConfirmWaypoint);
					}
					break;

				case CrsrSunriseDay:
				case CrsrSunriseMonth:
				case CrsrSunriseYear:
					// Normally, sunrise would be recalculated here...for now, leave
					//   functionality that sunrise is continuously calculated
					
					// Set cursor mode back to waypoint entry
					crsrMode = CrsrSunriseWpt;

					// Prepare for Sunrise waypoint ID edit mode
					crsrMode = CrsrSunriseWpt;
					iText = 0;
					iTextBound = 4;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptSunrise.id);
					break;
				}
				break;
			} // switch(page)
			break;

		case TriggerCLR:
			break;

		case TriggerCRSR:
			// Toggle CRSR mode
			crsr = !crsr;
			crsrElapsed = 0;
			crsrFlash = true;
			switch (page) {
				case NavCdi:
					crsrMode = CrsrNavCdiDistance;
					break;

				case NavPosition:
					crsrMode = CrsrNavPosProx;
					break;

				case NavMenu1:
 					crsrMode = CrsrNavMenu1TripPlan;
					break;

				case NavMenu2:
					crsrMode = CrsrNavMenu2ApprTime;
					break;

				case NavTripPlan:
					// Reset page back to menu with crsr enabled
					page = NavMenu1;
 					crsrMode = CrsrNavMenu1TripPlan;
					crsr = true;
					break;

				case NavDaltTas:
					// Reset page back to menu with crsr enabled
					page = NavMenu1;
 					crsrMode = CrsrNavMenu1DaltTas;
					crsr = true;
					break;

				case NavFuelPlan:
					// Reset page back to menu with crsr enabled
					page = NavMenu1;
 					crsrMode = CrsrNavMenu1FuelPlan;
					crsr = true;
					break;

				case NavWinds:
					// Reset page back to menu with crsr enabled
					page = NavMenu1;
 					crsrMode = CrsrNavMenu1Winds;
					crsr = true;
					break;

				case NavVnavPlan:
					// Reset page back to menu with crsr enabled
					page = NavMenu1;
 					crsrMode = CrsrNavMenu1VnavPlan;
					crsr = true;
					break;

				case NavChklistMenu:
					// Reset page back to menu with crsr enabled
					page = NavMenu1;
 					crsrMode = CrsrNavMenu1Chklist;
					crsr = true;
					break;

				case NavApprTime:
					// Reset page back to menu with crsr enabled
					page = NavMenu2;
					crsrMode = CrsrNavMenu2ApprTime;
					crsr = true;
					break;

				case NavClock:
					// Reset page back to menu with crsr enabled
					page = NavMenu2;
					crsrMode = CrsrNavMenu2Clock;
					crsr = true;
					break;

				case NavTripTime:
					// Reset page back to menu with crsr enabled
					page = NavMenu2;
					crsrMode = CrsrNavMenu2TripTime;
					crsr = true;
					break;

				case NavGpsPlan:
					// Reset page back to menu with crsr enabled
					page = NavMenu2;
					crsrMode = CrsrNavMenu2GpsPlan;
					crsr = true;
					break;

				case NavScheduler:
					// Reset page back to menu with crsr enabled
					page = NavMenu2;
					crsrMode = CrsrNavMenu2Scheduler;
					crsr = true;
					break;

				case NavSunrise:
					// Reset page back to menu with crsr enabled
					page = NavMenu2;
					crsrMode = CrsrNavMenu2Sunrise;
					crsr = true;
					break;

				case NavChklistDetail:
					// Revert back to checklist menu
					page = NavChklistMenu;
					crsr = true;
					break;
			}
			rc = true;
			break;

		case TriggerOuterRight:
			if (crsr) {
				switch (page) {
				case NavCdi:
					incNavCdiCrsrMode (crsrMode);
					break;

				case NavPosition:
					break;

				case NavMenu1:
					incNavMenu1CrsrMode (crsrMode);
					break;

				case NavMenu2:
					incNavMenu2CrsrMode (crsrMode);
					break;

				case NavClock:
					incNavClockCrsrMode (crsrMode);
					break;

				case NavApprTime:
					incNavApprTimeCrsrMode (crsrMode);
					break;

				case NavAutoStore:
					incNavAutoStoreCrsrMode (crsrMode);
					break;

				case NavScheduler:
					switch (crsrMode) {
					case CrsrNavSchedulerName:
						crsrMode = CrsrNavSchedulerTime;
						break;

					case CrsrNavSchedulerTime:
						crsrMode = CrsrNavSchedulerName;
						break;
					}
					break;

				case NavSunrise:
					switch (crsrMode) {
					case CrsrSunriseWpt:
						if (iText < iTextBound) iText++;
						break;

					case CrsrSunriseDay:
						// Advance to month
						crsrMode = CrsrSunriseMonth;
						break;

					case CrsrSunriseMonth:
						// Advance to year
						crsrMode = CrsrSunriseYear;
						break;

					case CrsrSunriseYear:
						// Cycle back to day
						crsrMode = CrsrSunriseDay;
						break;
					}
					break;
				} // switch (page)
			} else {
				// CRSR not active, outer knob changes NAV page
				switch (page) {
				case NavCdi:
				case NavPosition:
				case NavMenu1:
				case NavMenu2:
					page = incNavPage (page);
					elapsed = 0;
					updateNavDisplay ();
				}
			}
			rc = true;
			break;

		case TriggerOuterLeft:
			if (crsr) {
				// CRSR active
				switch (page) {
				case NavCdi:
					decNavCdiCrsrMode (crsrMode);
					break;

				case NavPosition:
					break;

				case NavMenu1:
					decNavMenu1CrsrMode (crsrMode);
					break;

				case NavMenu2:
					decNavMenu2CrsrMode (crsrMode);
					break;

				case NavClock:
					decNavClockCrsrMode (crsrMode);
					break;

				case NavApprTime:
					decNavApprTimeCrsrMode (crsrMode);
					break;

				case NavAutoStore:
					decNavAutoStoreCrsrMode (crsrMode);
					break;

				case NavScheduler:
					switch (crsrMode) {
					case CrsrNavSchedulerName:
						crsrMode = CrsrNavSchedulerTime;
						break;

					case CrsrNavSchedulerTime:
						crsrMode = CrsrNavSchedulerName;
						break;
					}
					break;

				case NavSunrise:
					switch (crsrMode) {
					case CrsrSunriseWpt:
						// Decrement text index for wpt edit field
						if (iText > 0) iText--;
						break;

					case CrsrSunriseDay:
						// Cycle around to year
						crsrMode = CrsrSunriseYear;
						break;

					case CrsrSunriseMonth:
						// Cycle back to day
						crsrMode = CrsrSunriseDay;
						break;

					case CrsrSunriseYear:
						// Cycle back to month
						crsrMode = CrsrSunriseMonth;
						break;
					}
					break;
				} // switch (page)
			} else {
				// CRSR not active, outer knob changes display object type
				switch (page) {
				case NavCdi:
				case NavPosition:
				case NavMenu1:
				case NavMenu2:
					page = decNavPage (page);
					elapsed = 0;
					updateNavDisplay ();
				}
			}
			rc = true;
			break;

		case TriggerInnerRight:
			if (crsr) {
				// CRSR active, inner knob rotates through details
				switch (page) {
				case NavCdi:
					switch (crsrMode) {
					case CrsrNavCdiDistance:
						incNavDistanceMode (disMode);
						break;

					case CrsrNavCdiSteer:
						incNavTrackMode (trkMode);
						break;

					case CrsrNavCdiEte:
						incNavEteMode (eteMode);
						break;
					}
					break;

				case NavPosition:
					incNavProximityMode (proximityMode);
					break;

				case NavMenu1:
					break;

				case NavMenu2:
					break;

				case NavClock:
					switch (crsrMode) {
					case CrsrClockTimeMode:
						// Toggle lcl/UTC time display
						pData->useUTC = !pData->useUTC;
						break;

					case CrsrClockLclHour:
						// Increment local time offset by 1 hour, up to maximum of +24h
						if (pData->lclOffset < 24) {
							pData->lclOffset += 1;
						}
						break;

					case CrsrClockLclMin:
						// Increment local time offset by 1 minute, wrap around from 0 to 59
						if (pData->lclOffset < 24) {
							pData->lclOffset += 1/60;
						}
					}
					break;

				case NavApprTime:
					switch (crsrMode) {
					case CrsrApprTimeDir:
						// Toggle up/down mode
						pData->apprTimerCountDown = !pData->apprTimerCountDown;
						break;

					case CrsrApprTimeHours:
						incOnes (apprTimeHour);
						pData->apprTimer += 3600;
						break;

					case CrsrApprTimeMinTens:
						incMinTens (apprTimeMin);
						pData->apprTimer += 600;
						break;

					case CrsrApprTimeMinOnes:
						incOnes (apprTimeMin);
						pData->apprTimer += 60;
						break;

					case CrsrApprTimeSecTens:
						incMinTens (apprTimeSec);
						pData->apprTimer += 10;
						break;

					case CrsrApprTimeSecOnes:
						incOnes (apprTimeSec);
						pData->apprTimer += 1;
						break;
					}
					break;

				case NavAutoStore:
					switch (crsrMode) {
					case CrsrNavAutoStoreId:
						break;

					case CrsrNavAutoStoreRte:
						if (rteAutoStore < 20) {
							// Increment auto store rte
							rteAutoStore++;
						} else {
							// Wrap around to no rte
							rteAutoStore = -1;
						}
						break;

					case CrsrNavAutoStoreOk:
						break;
					}
					break;

				case NavChklistMenu:
					// Increment checklist menu line select and/or base index
					if (iChecklist < pData->nChecklists-1) {
						// OK to advance to the next checklist
						iChecklist++;
						if (iChecklistSelect == 0) {
							// Current selecting first line, advance to second line
							iChecklistSelect = 1;
						}
					}
					break;

				case NavChklistDetail:
					// Increment checklist item line select and/or index
					if (iChecklistItem < pData->checklist[iChecklist].nItems-1) {
						// OK to advance to the next checklist
						iChecklistItem++;
						if (iChecklistItemSelect == 0) {
							// Current selecting first line, advance to second line
							iChecklistItemSelect = 1;
						}
					}
					break;

				case NavScheduler:
					// Increment scheduler selection and/or line select
					if (iScheduler < 8) {
						// OK to advance to the next checklist
						iScheduler++;
						if (iSchedulerSelect == 0) {
							// Current selecting first line, advance to second line
							iSchedulerSelect = 1;
						}
					}
					break;

				case NavSunrise:
					switch (crsrMode) {
					case CrsrSunriseWpt:
						// Increment text entry being edited
						incEntry (textValue[iText]);
						break;

					case CrsrSunriseDay:
						// Increment day, wrap around month-end
						if (dateSunrise.day < daysInMonth (dateSunrise.month, dateSunrise.year)) {
							dateSunrise.day++;
						} else {
							dateSunrise.day = 1;
						}
						break;

					case CrsrSunriseMonth:
						// Increment month, wrap around year-end
						dateSunrise.month++;
						if (dateSunrise.month > 12) dateSunrise.month = 1;
						break;

					case CrsrSunriseYear:
						// Increment year, wrap from 2089 to 1990
						dateSunrise.year++;
						if (dateSunrise.year > 2089) dateSunrise.year = 1990;
						break;
					}
					break;
				}
			} else {
				// CRSR is off
			}
			rc = true;
			break;

		case TriggerInnerLeft:
			if (crsr) {
				// CRSR active, inner knob rotates through details
				switch (page) {
				case NavCdi:
					switch (crsrMode) {
					case CrsrNavCdiDistance:
						decNavDistanceMode (disMode);
						break;

					case CrsrNavCdiSteer:
						decNavTrackMode (trkMode);
						break;

					case CrsrNavCdiEte:
						decNavEteMode (eteMode);
						break;
					}
					break;

				case NavPosition:
					decNavProximityMode (proximityMode);
					break;

				case NavMenu1:
					break;

				case NavMenu2:
					break;

				case NavApprTime:
					switch (crsrMode) {
					case CrsrApprTimeDir:
						// Toggle up/down mode
						pData->apprTimerCountDown = !pData->apprTimerCountDown;
						break;

					case CrsrApprTimeHours:
						decOnes (apprTimeHour);
						pData->apprTimer -= 3600;
						break;

					case CrsrApprTimeMinTens:
						decMinTens (apprTimeMin);
						pData->apprTimer -= 600;
						break;

					case CrsrApprTimeMinOnes:
						decOnes (apprTimeMin);
						pData->apprTimer -= 60;
						break;

					case CrsrApprTimeSecTens:
						decMinTens (apprTimeSec);
						pData->apprTimer -= 10;
						break;

					case CrsrApprTimeSecOnes:
						decOnes (apprTimeSec);
						pData->apprTimer -= 1;
						break;
					}
					break;

				case NavAutoStore:
					switch (crsrMode) {
					case CrsrNavAutoStoreId:
						break;

					case CrsrNavAutoStoreRte:
						if (rteAutoStore > -1) {
							// Decrement auto store rte
							rteAutoStore--;
						} else {
							// Wrap around to top rte
							rteAutoStore = 19;
						}
						break;

					case CrsrNavAutoStoreOk:
						break;
					}
					break;

				case NavChklistMenu:
					// Decrement checklist menu line select and/or base index
					if (iChecklist > 0) {
						// OK to go back to previous
						iChecklist--;
						if (iChecklistSelect == 1) {
							// Selection is second line, move back to first
							iChecklistSelect = 0;
						}
					}
					break;

				case NavChklistDetail:
					// Decrement checklist item line select and/or index
					if (iChecklistItem > 0) {
						// OK to go back to previous
						iChecklistItem--;
						if (iChecklistItemSelect == 1) {
							// Current selecting second line, move back to first
							iChecklistItemSelect = 0;
						}
					}
					break;

				case NavClock:
					switch (crsrMode) {
					case CrsrClockTimeMode:
						// Toggle lcl/UTC time display
						pData->useUTC = !pData->useUTC;
						break;

					case CrsrClockLclHour:
						// Decrement local time offset by 1 hour, up to maximum of -24h
						if (pData->lclOffset > -24) {
							pData->lclOffset -= 1;
						}
						break;

					case CrsrClockLclMin:
						// Decrement local time offset by 1 minute, wrap around from 0 to 59
						if (pData->lclOffset > -24) {
							pData->lclOffset -= 1/60;
						}
						break;

					}
					break;

				case NavScheduler:
					// Decrement scheduler selection and/or line select
					if (iScheduler > 0) {
						// OK to advance back to the prev checklist
						iScheduler--;
						if (iSchedulerSelect == 1) {
							// Current selecting second line, move back to first line
							iSchedulerSelect = 0;
						}
					}
					break;

				case NavSunrise:
					switch (crsrMode) {
					case CrsrSunriseWpt:
						// Decrement text entry being edited
						decEntry (textValue[iText]);
						break;

					case CrsrSunriseDay:
						// Decrement day, wrap around month-end
						if (dateSunrise.day > 1) {
							dateSunrise.day--;
						} else {
							dateSunrise.day = daysInMonth (dateSunrise.month, dateSunrise.year);
						}
						break;

					case CrsrSunriseMonth:
						// Decrement month, wrap around year-end
						dateSunrise.month--;
						if (dateSunrise.month < 1) dateSunrise.month = 12;
						break;

					case CrsrSunriseYear:
						// Decrement year, wrap from 1990 to 2089
						dateSunrise.year--;
						if (dateSunrise.year < 1990) dateSunrise.year = 2089;
						break;
					}
					break;
				}
			} else {
			}
			rc = true;
			break;
	}

	updateNavDisplay ();

	return rc;
}


void Nav::TimeSlice (float dT)
{
	static const float CRSR_ON_CYCLE  = (float)0.8;
	static const float CRSR_OFF_CYCLE = (float)0.2;
	static const float UpdateInterval = 1.0;
	static float elapsed = 0;

	// Increment cursor flash elapsed time
	if (crsr) {
		crsrElapsed += dT;
		if (crsrFlash) {
			// CRSR flash is on, delay for CRSR_ON_CYCLE time
			if (crsrElapsed > CRSR_ON_CYCLE) {
				crsrElapsed = 0;
				crsrFlash = false;
				if (crsr) updateNavDisplay ();
			}
		} else {
			// CRSR flash is off, delay for CRSR_OFF_CYCLE time
			if (crsrElapsed > CRSR_OFF_CYCLE) {
				crsrElapsed = 0;
				crsrFlash = true;
				if (crsr) updateNavDisplay ();
			}
		}
	}

	// Increment elapsed time since last real-time update
	elapsed += dT;
	if (elapsed > UpdateInterval) {
		elapsed = 0;
		updateNavDisplay ();
	}
}

static const int CrsrArptId = 1;
static const int CrsrArptComm = 2;
static const int CrsrArptRwy = 3;

void Nearest::UpdateNrstAirportDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	SGps150RealTimeData *pRealTime = pData->pRealTime;

	SNrstAirport *p = &airportList[airportSort[iAirport]];
	SFsimAirport *pAp = &p->airport;

	// Set line 1, Airport ID and altitude
	char sId[8];
	if (crsr && (crsrMode == CrsrArptId) && !crsrFlash) {
		strcpy (sId, "");
	} else {
		strcpy (sId, pAp->id);
	}

	char sAlt[8];
	formatAltitude ((float)pAp->pos.alt, pData->altUnits, sAlt);
	sprintf (pShared->line1, "nr%d apt %-4s %6s",
		iAirport+1, sId, sAlt);

	// Set line 2, Airport bearing, distance and current freq
	SFsimVector v = fsimGreatCirclePolar (pRealTime->userPos, pAp->pos);
	float brgTrue = RadToDeg ((float)v.y);
	float distNm = FeetToNm ((float)v.z);

	char sBearing[8];
	float brgMag = brgTrue - pData->magVar;
	Wrap360 (brgMag);
	int iBearing = (int)brgMag;
	sprintf (sBearing, "%03d~", iBearing);

	char sDistance[8];
	formatDistance (distNm, pData->navUnits, sDistance);

	char sType[8];
	if (crsr && !crsrFlash && crsrMode==CrsrArptComm) {
		strcpy (sType, "   ");
	} else {
		strcpy (sType, pAp->comm[p->iComm].name);
	}

	char sCommFreq[8];
	formatFrequency (pAp->comm[p->iComm].freq, sCommFreq);

	sprintf (pShared->line2, " %s%s %3s %s",
		sBearing, sDistance, sType, sCommFreq);

	// Set line 3, Airport current runway
	char sRwy[16];
	if (crsr && (crsrMode == CrsrArptRwy) && !crsrFlash) {
		strcpy (sRwy, "");
	} else {
		sprintf (sRwy, " rnwy %s", pAp->rwy[p->iRunway].id);
	}

	char sRwyLength[8];
	formatAltitude (pAp->rwy[p->iRunway].length, pData->altUnits, sRwyLength);

	sprintf (pShared->line3, "%13s %s",
		sRwy, sRwyLength);
}

void Nearest::UpdateNrstVorDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	SGps150RealTimeData *pRealTime = pData->pRealTime;

	SNrstNavaid *p = &vorList[vorSort[iVor]];
	SFsimNavaid *pVor = &p->navaid;

	char buffer[24];

	// Set line 1, VOR info
	char sId[7];
	if (crsr && !crsrFlash) {
		strcpy (sId, "");
	} else {
		strcpy (sId, pVor->id);
	}

	char sFreq[6];
	formatFrequency (pVor->freq, sFreq);

	sprintf (buffer, "nr%d vor %-6s %-5s", iVor+1, sId, sFreq);
	strcpy (pShared->line1, buffer);

	// Set line 2, VOR bearing, distance and info
	SFsimVector v = fsimGreatCirclePolar (pRealTime->userPos, pVor->pos);
	float brgTrue = RadToDeg ((float)v.y);
	float distNm = FeetToNm ((float)v.z);

	char sBearing[8];
	float brgMag = brgTrue - pData->magVar;
	Wrap360 (brgMag);
	int iBearing = (int)brgMag;
	sprintf (sBearing, "%03d~", iBearing);

	char sDistance[8];
	formatDistance (distNm, pData->navUnits, sDistance);

	sprintf (buffer, " %s %s ", sBearing, sDistance);

	if (pVor->type & FSIM_NAVAID_TYPE_TACAN) {
		strcat (buffer, "tacan");
	} else if (pVor->type & FSIM_NAVAID_TYPE_DME) {
		strcat (buffer, "dme");
	}

	sprintf (pShared->line2, " %s %s", sBearing, sDistance);

	// Set line 3, VOR name
	strncpy (pShared->line3, pVor->name, 20);
}

void Nearest::UpdateNrstNdbDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	SGps150RealTimeData *pRealTime = pData->pRealTime;

	SNrstNavaid *p = &ndbList[ndbSort[iNdb]];
	SFsimNavaid *pNdb = &p->navaid;

	char buffer[24];

	// Set line 1, NDB info
	char sId[7];
	if (crsr && !crsrFlash) {
		strcpy (sId, "");
	} else {
		strcpy (sId, pNdb->id);
	}

	char sFreq[6];
	formatFrequency (pNdb->freq, sFreq);
	sprintf (buffer, "nr%d ndb %-6s %-5s", iNdb+1, sId, sFreq);
	strcpy (pShared->line1, buffer);

	// Set line 2, NDB bearing, distance and info
	SFsimVector v = fsimGreatCirclePolar (pRealTime->userPos, pNdb->pos);
	float brgTrue = RadToDeg ((float)v.y);
	float distNm = FeetToNm ((float)v.z);

	char sBearing[8];
	float brgMag = brgTrue - pData->magVar;
	Wrap360 (brgMag);
	int iBearing = (int)brgMag;
	sprintf (sBearing, "%03d~", iBearing);

	char sDistance[8];
	formatDistance (distNm, pData->navUnits, sDistance);

	sprintf (buffer, " %s %s ", sBearing, sDistance);

	if (pNdb->type & FSIM_NAVAID_TYPE_DME) {
		strcat (buffer, "dme");
	}

	sprintf (pShared->line2, " %s %s", sBearing, sDistance);

	// Set line 3, NDB name
	strncpy (pShared->line3, pNdb->name, 20);
}


void Nearest::UpdateNrstFixDisplay (void)
{
	char buffer[24];

	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	SGps150RealTimeData *pRealTime = pData->pRealTime;

	int i1 = iFix;
	int i2 = ((iFix + 1) == nFix) ? 0 : (iFix+1);

	SNrstIntersection *p1 = &fixList[fixSort[i1]];
	SFsimIntersection *pFix1 = &p1->intersection;
	SNrstIntersection *p2 = &fixList[fixSort[i2]];
	SFsimIntersection *pFix2 = &p2->intersection;

	// Set line 1, Fix info
	sprintf (buffer, "Nearest intersectns");
	strcpy (pShared->line1, buffer);

	// Set line 2, fix 1 info
	char sName[6];
	memset (sName, 0, 6);
	strncpy (sName, pFix1->name, 5);

	SFsimVector v = fsimGreatCirclePolar (pRealTime->userPos, pFix1->pos);
	float brgTrue = RadToDeg ((float)v.y);
	float distNm = FeetToNm ((float)v.z);

	char sBearing[8];
	float brgMag = brgTrue - pData->magVar;
	Wrap360 (brgMag);
	int iBearing = (int)brgMag;
	sprintf (sBearing, "%03d~", iBearing);

	char sDistance[8];
	formatDistance (distNm, pData->navUnits, sDistance);

	if (crsr && !crsrFlash) {
		strcpy (buffer, "");
	} else {
		sprintf (buffer, " %1d %5s  %4s%4s",
			i1+1, sName, sBearing, sDistance);
	}
	strcpy (pShared->line2, buffer);

	// Set line 3, fix 2 info
	memset (sName, 0, 6);
	strncpy (sName, pFix2->name, 5);

	v = fsimGreatCirclePolar (pRealTime->userPos, pFix2->pos);
	brgTrue = RadToDeg ((float)v.y);
	distNm = FeetToNm ((float)v.z);
	brgMag = brgTrue - pData->magVar;
	Wrap360 (brgMag);
	iBearing = (int)brgMag;
	sprintf (sBearing, "%03d~", iBearing);

	formatDistance (distNm, pData->navUnits, sDistance);

	sprintf (buffer, " %1d %5s  %4s%4s",
		i2+1, sName, sBearing, sDistance);
	strcpy (pShared->line3, buffer);
}

void Nearest::UpdateNrstFssDisplay (void)
{
	char buffer[24];

	// Set line 1, FSS info
	sprintf (buffer, "nr%1d fss %s", iFss+1, fssList[iFss].comm.name);
	strcpy (pShared->line1, buffer);

	// Set line 2, FSS position
	char sFreq[8];
	formatFrequency (fssList[iFss].comm.freq, sFreq);
	sprintf (pShared->line2, "                %s", sFreq);

	// Set line 3, FSS name/location
	strcpy (buffer, "");
	strcpy (pShared->line3, buffer);
}


void Nearest::UpdateNrstDisplay (void)
{
	switch (page) {
		case NrstAirport:
			UpdateNrstAirportDisplay ();
			break;

		case NrstVor:
			UpdateNrstVorDisplay ();
			break;

		case NrstNdb:
			UpdateNrstNdbDisplay ();
			break;

		case NrstFix:
			UpdateNrstFixDisplay ();
			break;

		case NrstFss:
			UpdateNrstFssDisplay ();
			break;
	}
}

static void incNrstType (ENrstPage &page)
{
	switch (page) {
		case NrstAirport:
			page = NrstVor;
			break;

		case NrstVor:
			page = NrstNdb;
			break;

		case NrstNdb:
			page = NrstFix;
			break;

		case NrstFix:
			page = NrstFss;
			break;

		case NrstFss:
			page = NrstAirport;
			break;
	}
}

static void decNrstType (ENrstPage &page)
{
	switch (page) {
		case NrstAirport:
			page = NrstFss;
			break;

		case NrstVor:
			page = NrstAirport;
			break;

		case NrstNdb:
			page = NrstVor;
			break;

		case NrstFix:
			page = NrstNdb;
			break;

		case NrstFss:
			page = NrstFix;
			break;
	}
}

static void CopyNearestAirport (SFsimAirport *pAirport,
								SNrstAirport *pNearest,
								float         distance)
{
	pNearest->distance = distance;
	memcpy (&pNearest->airport, pAirport, sizeof(SFsimAirport));

	//pNearest->pos = pAirport->pos;
	//formatAirportId (pAirport, pNearest->id);

	//// Copy runway data
	//SFsimRunway *pNextRwy = pAirport->runways;
	//int nRwys = 0;
	//SNrstRunway *pNrstRwy;
	//while (pNextRwy && (nRwys < 16)) {

	//	pNrstRwy = &pNearest->rwy[nRwys];
	//	pNrstRwy->length = pNextRwy->length;
	//	sprintf (pNrstRwy->id, "%3s/%-3s",
 //			pNextRwy->base.id, pNextRwy->recip.id);

	//	nRwys++;
	//	pNextRwy = pNextRwy->next;
	//}
	//pNearest->nRunways = nRwys;
	//pNearest->iRunway = 0;

	//// Copy comms data
	//SFsimComm *pNextComm = pAirport->comms;
	//int nComms = 0;
	//SNrstComm *pNrstComm;
	//while (pNextComm && (nComms < 16)) {

	//	pNrstComm = &pNearest->comm[nComms];
	//	pNrstComm->freq = pNextComm->freq[0];
	//	formatCommType (pNextComm->type, pNrstComm->name);

	//	nComms++;
	//	pNextComm = pNextComm->next;
	//}
	//pNearest->nComms = nComms;
	//pNearest->iComm = 0;
}

static void CopyNearestNavaid (SFsimNavaid   *pNavaid,
							   SNrstNavaid   *pNearest,
							   float         distance)
{
	pNearest->distance = distance;
	memcpy (&pNearest->navaid, pNavaid, sizeof(SFsimNavaid));

//	pNearest->pos = pNavaid->pos;
//	pNearest->freq = pNavaid->freq;
//	pNearest->type = pNavaid->type;
//	strcpy (pNearest->id, pNavaid->id);
//	strcpy (pNearest->name, pNavaid->name);
}


static void CopyNearestFix (SFsimIntersection   *pFix,
							SNrstIntersection   *pNearest,
							float               distance)
{
	pNearest->distance = distance;
	memcpy (&pNearest->intersection, pFix, sizeof(SFsimIntersection));

	//pNearest->pos.lat  = pFix->pos.lat;
	//pNearest->pos.lon  = pFix->pos.lon;
	//pNearest->pos.alt  = pFix->pos.alt;
	//strcpy (pNearest->name, pFix->name);
}


static void CopyNearestFss (SFsimComm   *pFss,
							SNrstComm   *pNearest,
							float       distance)
{
	pNearest->distance = distance;
	memcpy (&pNearest->comm, pFss, sizeof(SFsimComm));

	//pNearest->freq = pComm->freq[0];
	//strcpy (pNearest->name, pComm->name);

}


void Nearest::UpdateAirportList (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	int nAirports = 0;				// Number added to nearest list
	int iMaxDistance = 0;			// Index of farthest airport in nearest list
	float maxDistance = (float)0;	// Distance of farthest airport in nearest list

	// Get a list of the local airports.  The behaviour of this function is
	//   sim platform specific, but we assume that at least the nine closest
	//   airports, up to and including all within 200nm, is returned.
	SFsimAirportList *pList = fsimLocalAirports ();

	// Transfer the nine nearest airports to the summary list
	for (int i=0; i<pList->nAirports; i++) {

		// Calculate distance to this airport
		SFsimVector v = fsimSubtractPosition (pList->pAirport[i].pos, pRealTime->userPos);
		float d = FeetToNm((float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z));

		if (nAirports == 9) {
			// Nearest list is full, see if this airport is closer than the farthest
			//   one in the list
			if (d < maxDistance) {
				// This is closer than the current farthest in the list
				CopyNearestAirport (&pList->pAirport[i], &airportList[iMaxDistance], d);

				// Recalculate index of farthest airport
				maxDistance = (float)0;
				for (int j=0; j<9; j++) {
					if (airportList[j].distance > maxDistance) {
						iMaxDistance = j;
						maxDistance = airportList[j].distance;
					}
				}
			}
		} else {
			// Summary list is not full yet, so add this element
			//   and increment the total # in the list
			CopyNearestAirport (&pList->pAirport[i], &airportList[nAirports], d);

			if (d > maxDistance) {
				iMaxDistance = nAirports;
				maxDistance = d;
			}

			// Update tracking list of airport distances
			nAirports++;
		}
	}

	// Free airport list created in fsimLocalAirports
	fsimFreeAirportList (pList);

	// Create list of sort indexes
	float min = (float)0;
	int k = 0;
	for (i=0; i<9; i++) {
		float max = (float)200;
		for (int j=0; j<9; j++) {
			float d = airportList[j].distance;
			if ((d > min) && (d < max)) {
				k = j;
				max = d;
			}
		}
		airportSort[i] = k;
		min = airportList[k].distance;
	}
}


//
// Updates the lists of nearest VORs and NDBs
//
void Nearest::UpdateNavaidList (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	// Get a list of the local navaids.  The behaviour of this function is
	//   sim platform specific, but we assume that at least the nine closest
	//   VORs and NDBs, up to and including all within 200nm, is returned.
	SFsimNavaidList *pList = fsimLocalNavaids ();
	nVor = 0;
	nNdb = 0;

	// Initialize variables used to filter the list for the nine nearest
	//   VORs and NDBs.
	int iMaxVorDistance, iMaxNdbDistance;
	float maxVorDistance = (float)0;
	float maxNdbDistance = (float)0;

	for (int i=0; i<pList->nNavaids; i++) {
		SFsimNavaid *pNextNavaid = &pList->pNavaid[i];

		// Calculate navaid distance
		SFsimVector v = fsimSubtractPosition (pNextNavaid->pos, pRealTime->userPos);
		float d = FeetToNm((float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z));
	
		if (pNextNavaid->type & FSIM_NAVAID_TYPE_VOR) {
			if (nVor == 9) {
				// Summary list is full, see if this VOR is closer than one
				//   already in the list
				if (d < maxVorDistance) {
					// This is closer than the current farthest in the list
					CopyNearestNavaid (pNextNavaid, &vorList[iMaxVorDistance], d);

					// Recalculate index of farthest airport
					maxVorDistance = (float)0;
					for (int i=0; i<9; i++) {
						if (vorList[i].distance > maxNdbDistance) {
							iMaxVorDistance = i;
							maxVorDistance = vorList[i].distance;
						}
					}
				}
			} else {
				// Summary list is not full yet, so add this element
				//   and increment the total # in the list
				CopyNearestNavaid (pNextNavaid, &vorList[nVor], d);

				if (d > maxVorDistance) {
					iMaxVorDistance = nVor;
					maxVorDistance = d;
				}

				// Increment number of VORs in the summary list
				nVor++;
			}

		} else if (pNextNavaid->type & FSIM_NAVAID_TYPE_NDB) {

			if (nNdb == 9) {
				// Summary list is full, see if this NDB is closer than one
				//   already in the list
				if (d < maxNdbDistance) {
					// This is closer than the current farthest in the list
					CopyNearestNavaid (pNextNavaid, &ndbList[iMaxNdbDistance], d);

					// Recalculate index of farthest airport
					maxNdbDistance = (float)0;
					for (int i=0; i<9; i++) {
						if (ndbList[i].distance > maxNdbDistance) {
							iMaxNdbDistance = i;
							maxNdbDistance = ndbList[i].distance;
						}
					}
				}
			} else {
				// Summary list is not full yet, so add this element
				//   and increment the total # in the list
				CopyNearestNavaid (pNextNavaid, &ndbList[nNdb], d);

				if (d > maxNdbDistance) {
					iMaxNdbDistance = nNdb;
					maxNdbDistance = d;
				}

				// Increment number of VORs in the summary list
				nNdb++;
			}
		}
	}

	// Create list of sorted VOR indices
	float min = (float)0;
	int k = 0;
	for (i=0; i<9; i++) {
		float max = (float)200;
		for (int j=0; j<9; j++) {
			float d = vorList[j].distance;
			if ((d > min) && (d < max)) {
				k = j;
				max = d;
			}
		}
		vorSort[i] = k;
		min = vorList[k].distance;
	}

	// Create list of sorted NDB indices
	min = (float)0;
	k = 0;
	for (i=0; i<9; i++) {
		float max = (float)200;
		for (int j=0; j<9; j++) {
			float d = ndbList[j].distance;
			if ((d > min) && (d < max)) {
				k = j;
				max = d;
			}
		}
		ndbSort[i] = k;
		min = ndbList[k].distance;
	}

	// Free list of navaids returned by fsimLocalNavaids
	fsimFreeNavaidList (pList);
}


void Nearest::UpdateFixList (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	// Get list of local intersection waypoints
	SFsimIntersectionList *pList = 	fsimLocalIntersections();

	// Initialize variables used to filter the list for the nine nearest
	//   VORs and NDBs.
	int iMaxDistance;
	float maxDistance = (float)0;

	// Transfer 9 nearest waypoints to waypoint summary list
	nFix = 0;	
	for (int i=0; i<pList->nIntersections; i++) {
		SFsimIntersection *pNextIntersection = &pList->pIntersection[i];

		// Calculate distance
		SFsimVector v = fsimSubtractPosition (pNextIntersection->pos, pRealTime->userPos);
		float d = FeetToNm((float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z));

		if (pNextIntersection->type == FSIM_WAYPOINT_TYPE_NAMED) {
			if (nFix == 9) {
				// Summary list is full, see if this waypoint is closer than one
				//   already in the list
				if (d < maxDistance) {
					// This is closer than the current farthest in the list
					CopyNearestFix (pNextIntersection, &fixList[iMaxDistance], d);

					// Recalculate index of farthest waypoint
					maxDistance = (float)0;
					for (int i=0; i<9; i++) {
						if (fixList[i].distance > maxDistance) {
							iMaxDistance = i;
							maxDistance = fixList[i].distance;
						}
					}
				}
			} else {
				// Summary list is not full yet, so add this element
				//   and increment the total # in the list
				CopyNearestFix (pNextIntersection, &fixList[nFix], d);

				if (d > maxDistance) {
					iMaxDistance = nFix;
					maxDistance = d;
				}

				// Update tracking list of waypoint distances
				nFix++;
			}
		}
	}

	// Free list of intersections returned by fsimLocalIntersections
	fsimFreeIntersectionList (pList);

	// Create list of sort indexes
	float min = (float)0;
	int k = 0;
	for (i=0; i<9; i++) {
		float max = (float)200;
		for (int j=0; j<9; j++) {
			float d = fixList[j].distance;
			if ((d > min) && (d < max)) {
				k = j;
				max = d;
			}
		}
		fixSort[i] = k;
		min = fixList[k].distance;
	}
}


void Nearest::UpdateFssList (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	// Get list of local comm facilities
	SFsimCommList *pList = fsimLocalComms ();

	// Transfer 2 nearest FSS to summary list
	nFss = 0;
	for (int i=0; i<pList->nComms; i++) {
		SFsimComm *pNextComm = &pList->pComm[i];

		// Calculate distance
		SFsimVector v = fsimSubtractPosition (pNextComm->pos, pRealTime->userPos);
		float d = FeetToNm((float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z));

		if (pNextComm->type == 32) {
			CopyNearestFss (pNextComm, &fssList[nFss], d);
			nFss++;
		}

		// If summary list is full then break
		if (nFss == 2) {
			break;
		}
	}

	// Free list of comms returned by fsimLocalComms
	fsimFreeCommList (pList);
}


void Nearest::Activate (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	// Check whether activation is due to waypoint confirmation
	if (wpConfirmPending) {
		wpConfirmPending = false;

		// We are waiting for a waypoint confirmation
		if (pData->wpActive.confirmed) {

			// Transfer D->TO waypoint to Active Route
			float gs = fsimUserGroundSpeed ();
			gs = FpsToKt (gs);
			clearRteActive (pData->rteActive);
			directToRte (pRealTime->userPos, pData->wpActive, &pData->rteActive, pData->autoLegSelect);

			// Active waypoint has been confirmed
			pData->fsm.HandleTrigger (TriggerNAV);
		}
	} else {
		// Set indicator light in shared data
		pShared->liteNrst = true;

		// Update list of 9 nearest airports
		UpdateAirportList ();

		// Update list of 9 nearest VOR and NDB navaids
		UpdateNavaidList ();

		// update list of 9 nearest fixes
		UpdateFixList ();

		// Update list of 2 nearest FSS COMM freqs
		UpdateFssList ();
	
		// Initialize display mode to Airports, and init display indices for all types
		page = NrstAirport;
		crsr = false;

		iAirport = 0;
		iVor = 0;
		iNdb = 0;
		iFix = 0;
		iFss = 0;

		UpdateNrstDisplay ();
	}
}


void Nearest::Deactivate (void)
{
	// Clear indicator light in shared data
	pShared->liteNrst = false;
}


bool Nearest::HandleTrigger (FsmTrigger t)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	bool rc = false;

	switch (t) {
	case TriggerD_TO:
		// DirectTo
		switch (page) {
		case NrstAirport:
			{
				// Initialize pending airport data
				wpConfirmPending = true;
				pData->replyTrigger = TriggerNRST;

				SRouteWaypoint &wp = pData->wpActive;
				SNrstAirport &ap = airportList[airportSort[iAirport]];
				wp.type = WptAirport;
				strcpy (wp.id, ap.airport.id);
				wp.confirmed = false;
				wp.direct = true;

				// Transition to the Waypoint Confirm state
				pData->fsm.HandleTrigger (TriggerConfirmWaypoint);
			}
			break;

		case NrstVor:
			{
				// Initialize pending VOR data
				wpConfirmPending = true;
				pData->replyTrigger = TriggerNRST;

				SRouteWaypoint &wp = pData->wpActive;
				SNrstNavaid &vor = vorList[vorSort[iVor]];
				wp.type = WptVor;
				strcpy (wp.id, vor.navaid.id);
				wp.confirmed = false;
				wp.direct = true;

				// Transition to the Waypoint Confirm state
				pData->fsm.HandleTrigger (TriggerConfirmWaypoint);
			}
			break;

		case NrstNdb:
			{
				// Initialize pending NDB data
				wpConfirmPending = true;
				pData->replyTrigger = TriggerNRST;

				SRouteWaypoint &wp = pData->wpActive;
				SNrstNavaid &ndb = ndbList[ndbSort[iNdb]];
				wp.type = WptNdb;
				strcpy (wp.id, ndb.navaid.id);
				wp.confirmed = false;
				wp.direct = true;

				// Transition to the Waypoint Confirm state
				pData->fsm.HandleTrigger (TriggerConfirmWaypoint);
			}
			break;
		}
		rc = true;
		break;

	case TriggerCRSR:
		crsr = !crsr;
		crsrElapsed = 0;
		crsrFlash = true;
		crsrMode = CrsrArptId;  // Only applicable in airport mode
		rc = true;
		break;

	case TriggerOuterRight:
		if (crsr) {
			switch (page) {
			case WptAirport:
				// Cycle right through id, comm and rwy 
				crsrMode++;
				if (crsrMode > CrsrArptRwy) crsrMode = CrsrArptId;
				break;
			}
		} else {
			// CRSR not active, outer knob changes display object type
			incNrstType (page);
		}
		rc = true;
		break;

	case TriggerOuterLeft:
		if (crsr) {
			// CRSR active, outer knob rotates through object details
			switch (page) {
			case WptAirport:
				// Cycle right through id, comm and rwy 
				crsrMode--;
				if (crsrMode == 0) crsrMode = CrsrArptRwy;
				break;
			}
		} else {
			// CRSR not active, outer knob changes display object type
			decNrstType (page);
		}
		rc = true;
		break;

	case TriggerInnerRight:
		if (crsr) {
			// CRSR active, inner knob rotates through details
			switch (page) {
			case WptAirport:
				switch (crsrMode) {
				case CrsrArptId:
					// Do nothing
					break;

				case CrsrArptComm:
					{
						SNrstAirport *p;
						p = &airportList[airportSort[iAirport]];
						int i = p->iComm;
						i++;
						if (i >= p->airport.nComm)
							i = 0;
						p->iComm = i;
					}
					break;

				case CrsrArptRwy:
					{
						SNrstAirport *p;
						p = &airportList[airportSort[iAirport]];
						int i = p->iRunway;
						i++;
						if (i >= p->airport.nRwy)
							i = 0;
						p->iRunway = i;
					}
					break;
				}
			}
		} else {
			// CRSR not active, inner knob cycles through nearest objects
			switch (page) {
				case WptAirport:
					iAirport = (iAirport + 1) % nAirports;
					break;

				case WptVor:
					iVor = (iVor + 1) % nVor;
					break;

				case WptNdb:
					iNdb = (iNdb + 1) % nNdb;
					break;

				case WptFix:
					iFix = (iFix + 1) % nFix;
					break;

				case WptFss:
					iFss = (iFss + 1) % nFss;
					break;
			}
		}
		rc = true;
		break;

	case TriggerInnerLeft:
		if (crsr) {
			// CRSR active, inner knob rotates through details
			switch (page) {
			case WptAirport:
				switch (crsrMode) {
				case CrsrArptId:
					// Do nothing
					break;

				case CrsrArptComm:
					{
						SNrstAirport *p;
						p = &airportList[airportSort[iAirport]];
						int i = p->iComm;
						i--;
						if (i == 0)
							i = p->airport.nComm - 1;
						p->iComm = i;
					}
					break;

				case CrsrArptRwy:
					{
						SNrstAirport *p;
						p = &airportList[airportSort[iAirport]];
						int i = p->iRunway;
						i--;
						if (i < 0)
							i = p->airport.nRwy - 1;
						p->iRunway = i;
					}
					break;
				}
			}
		} else {
			// CRSR not active, inner knob cycles through nearest objects
			switch (page) {
				case WptAirport:
					iAirport--;
					if ((iAirport < 0) && (nAirports > 0))
						iAirport = nAirports-1;
					break;

				case WptVor:
					iVor--;
					if ((iVor < 0) && (nVor > 0))
						iVor = nVor-1;
					break;

				case WptNdb:
					iNdb--;
					if ((iNdb < 0) && (nNdb > 0))
						iNdb = nNdb-1;
					break;

				case WptFix:
					iFix--;
					if ((iFix < 0) && (nFix > 0))
						iFix = nFix-1;
					break;

				case WptFss:
					iFss--;
					if ((iFss < 0) && (nFss > 0))
						iFss = nFss-1;
					break;
			}
		}
		rc = true;
		break;
	}

	// Update display contents
	UpdateNrstDisplay ();

	return rc;
}


void Nearest::TimeSlice (float dT)
{
	static const float CRSR_ON_CYCLE  = (float)0.8;
	static const float CRSR_OFF_CYCLE = (float)0.2;
	static const float UpdateInterval = 1.0;
	static float elapsed = 0;

	// Increment elapsed time since last real-time update
	elapsed += dT;
	if (elapsed > UpdateInterval) {
		elapsed = 0;
		UpdateNrstDisplay ();
	}

	// Increment cursor flash elapsed time
	if (crsr) {
		crsrElapsed += dT;
		if (crsrFlash) {
			// CRSR flash is on, delay for CRSR_ON_CYCLE time
			if (crsrElapsed > CRSR_ON_CYCLE) {
				crsrElapsed = 0;
				crsrFlash = false;
				UpdateNrstDisplay ();
			}
		} else {
			// CRSR flash is off, delay for CRSR_OFF_CYCLE time
			if (crsrElapsed > CRSR_OFF_CYCLE) {
				crsrElapsed = 0;
				crsrFlash = true;
				UpdateNrstDisplay ();
			}
		}
	}
}


//
// PowerUp state
//
//const float PowerUp::SelfTestInterval_m = 4.0;
const float PowerUp::SelfTestInterval_m = 1.0;

void PowerUp::UpdateDisplay1 (void)
{
	strcpy (pShared->line1, " GPS150 for Fly! II ");
	strcpy (pShared->line2, " (c) 2002 PMDG Inc. ");

	if (crsrFlash) {
		sprintf (pShared->line3, "Performing self test");
	} else {
		strcpy (pShared->line3, "");
	}
}


void PowerUp::UpdateDisplay2 (void)
{
	strcpy (pShared->line1, " GPS150 (TM) Garmin ");
	strcpy (pShared->line2, " International Inc. ");

	if (crsrFlash) {
		sprintf (pShared->line3, "Performing self test");
	} else {
		strcpy (pShared->line3, "");
	}
}


void PowerUp::UpdateDisplay3 (void)
{
	strcpy (pShared->line1,     " GPS150 for Fly! II ");
	
	if (crsrFlash) {
		strcpy (pShared->line2, "Do not use for real-");
		strcpy (pShared->line3, "  world navigation  ");
	} else {
		strcpy (pShared->line2, "");
		strcpy (pShared->line3, "");
	}
}



void PowerUp::UpdateDisplay (void)
{
	switch (page) {
	case 1:
		UpdateDisplay1 ();
		break;

	case 2:
		UpdateDisplay2 ();
		break;

	case 3:
		UpdateDisplay3 ();
		break;
	}
}

void PowerUp::Activate (void)
{
	crsrFlash = true;
	crsrElapsed = 0;

	// Initialize to page 1
	page = 1;
	elapsed = 0;

	UpdateDisplay ();
}

void PowerUp::Deactivate (void)
{
}

void PowerUp::TimeSlice (float dT)
{
	static const float CRSR_ON_CYCLE  = (float)0.8;
	static const float CRSR_OFF_CYCLE = (float)0.2;

	// Initialize pointer to application data for the FSM
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// When power-on time (updated in the main DLL timeslice callback) has
	//   exceeded the threshold, then transition to the next page
	elapsed += dT;
	if (elapsed > SelfTestInterval_m) {
		elapsed = 0;
		page++;
		if (page > 3) {
			// Self test expiry trigger
			pData->fsm.HandleTrigger (TriggerSelfTestExpiry);
		}
	}

	// Increment cursor flash elapsed time
	crsrElapsed += dT;
	if (crsrFlash) {
		// CRSR flash is on, delay for CRSR_ON_CYCLE time
		if (crsrElapsed > CRSR_ON_CYCLE) {
			crsrElapsed = 0;
			crsrFlash = false;
			UpdateDisplay ();
		}
	} else {
		// CRSR flash is off, delay for CRSR_OFF_CYCLE time
		if (crsrElapsed > CRSR_OFF_CYCLE) {
			crsrElapsed = 0;
			crsrFlash = true;
			UpdateDisplay ();
		}
	}
}

//
// PowerOff state
//
void PowerOff::Activate (void)
{
	pShared->onOff = 0;
	
	// Clear all text lines
	strcpy (pShared->line1, "");
	strcpy (pShared->line2, "");
	strcpy (pShared->line3, "");
}

void PowerOff::Deactivate (void)
{
	// Turn power "on" for the gauge
	pShared->onOff = 1;
}

//
// Route state
//
static const int CrsrActiveDistance = 0;
static const int CrsrActiveTime = 1;

static const int CrsrRteActivate = 0;
static const int CrsrRteReverse = 1;
static const int CrsrRteEdit = 2;
static const int CrsrRteDelete = 3;
static const int CrsrRteCopy = 4;
static const int CrsrRteCPA = 5;
static const int CrsrRteSearch = 6;

static const int CrsrSearchOffset    = 10;
static const int CrsrSearchIncrement = 11;
static const int CrsrSearchOk        = 12;

static const int CrsrCpaId           = 20;
static const int CrsrCpaOk           = 21;


void Route::FormatRteWpt (int leg, char *buffer)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	// Convenience pointer to active route and leg endpoints
	SRteActive *pRoute = &pData->rteActive;
	SRteLeg *pLeg = &pRoute->leg[leg];
	SRteWaypoint *pWpt = &pRoute->stored.waypoint[leg];
	SRteWaypoint *pNext = &pRoute->stored.waypoint[leg+1];

	// If the waypoint to display is out of range of the active
	//   flight plan, display nothing.
	if ((leg < 0) || (leg >= pRoute->nLegs)) {
		strcpy (buffer, "");
		return;
	}

	// Format destination waypoint name for this leg
	char sWptName[6];
	formatWaypointName (pNext->name, sWptName);

	// Determine waypoint intro character
	char cIntro;
	if (leg == pRoute->iLeg) {
		cIntro = '}';
	} else {
		cIntro = ':';
	}

	// Format distance field
	float d = 0;
	switch (distMode) {
	case RteDistCum:
		d = pLeg->cumDistance;
		break;

	case RteDistLeg:
		d = pLeg->legDistance;
		break;
	}
	char sDistance[16];
	formatDistance (d, pData->navUnits, sDistance);

	// Calculate ete in hours (NM / KTS)
	float ete = d / pRealTime->gs;
	
	// TODO: Add ete to current time for eta
	float eta = ete;

	// TODO: Convert ETA into local time if necessary
	if (!pData->useUTC) {
	}

	// Format time/dtk field
	char sTime[10];
	switch (timeMode) {
	case RteTimeEte:
		formatTime (ete, sTime);
		break;

	case RteTimeEta:
		formatTime (eta, sTime);
		break;

	case RteTimeDtk:
		formatBearing (pLeg->dtk, sTime);
		break;

	default:
		strcpy (sTime, "");
	}

	// Format waypoint details
	sprintf (buffer, "%2d%c%s %-6s %s",
		leg, cIntro, sWptName, sDistance, sTime);
}


void Route::UpdateRteActiveDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	char buffer[25];
	SRteActive *pRteActive = &pData->rteActive;

	int wptFrom = pRteActive->iLeg;
	int wptTo = pRteActive->iLeg+1;

	// Format from waypoint
	char sActiveFrom[6];
	formatWaypointName (pRteActive->stored.waypoint[wptFrom].name, sActiveFrom);

	// Format To waypoint
	char sActiveTo[6];
	formatWaypointName (pRteActive->stored.waypoint[wptTo].name, sActiveTo);

	// Format distance mode
	char sDistMode[5];
	if ((crsrMode == CrsrActiveDistance) && !crsrFlash) {
		strcpy (sDistMode, "");
	} else {
		switch (distMode) {
		case RteDistCum:
			strcpy (sDistMode, "cum");
			break;

		case RteDistLeg:
			strcpy (sDistMode, "leg");
			break;

		default:
			strcpy (sDistMode, "???");
		}
	}

	// Format time mode
	char sTimeMode[5];
	if ((crsrMode == CrsrActiveTime) && !crsrFlash) {
		strcpy (sTimeMode, "");
	} else {
		switch (timeMode) {
		case RteTimeEte:
			strcpy (sTimeMode, "ete");
			break;

		case RteTimeEta:
			strcpy (sTimeMode, "eta");
			break;

		case RteTimeDtk:
			strcpy (sTimeMode, "dtk");
			break;

		default:
			strcpy (sTimeMode, "");
		}
	}

	// Line 1, from/to waypoints, distMode and timeMode
	sprintf (buffer, "%-5s}%-5s  %3s %3s",
		sActiveFrom, sActiveTo, sDistMode, sTimeMode);
	strcpy (pShared->line1, buffer);

	// Line 2, wpt details for selected wpt
	FormatRteWpt (iLeg, buffer);
	strcpy (pShared->line2, buffer);

	// Line 3, wpt details for next wpt
	FormatRteWpt (iLeg+1, buffer);
	strcpy (pShared->line3, buffer);
}


void Route::UpdateRteCatalogDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	char buffer[25];

	// Set cursor prompt
	char sPrompt[16];
	if (crsr) {
		if (crsrFlash) {
			switch (crsrMode) {
			case CrsrRteActivate:
				// Change prompt to "transfer" when selected route is #0, to indicate
				//   that route will be transferred from active Fly! flight plan
				if (iRte == 0) {
					strcpy (sPrompt, "transfer?");
				} else {
					strcpy (sPrompt, "activate?");
				}
				break;

			case CrsrRteReverse:
				strcpy (sPrompt, "reverse?");
				break;

			case CrsrRteEdit:
				strcpy (sPrompt, "edit?");
				break;

			case CrsrRteDelete:
				strcpy (sPrompt, "delete?");
				break;

			case CrsrRteCopy:
				strcpy (sPrompt, "copy?");
				break;

			case CrsrRteCPA:
				strcpy (sPrompt, "cpa?");
				break;

			case CrsrRteSearch:
				strcpy (sPrompt, "search?");
				break;

			default:
				strcpy (sPrompt, "");
			}
		}
	} else {
		// Cursor mode not active, don't display any prompt
		strcpy (sPrompt, "");
	}

	// Format first line, rte # and cursor mode
	sprintf (buffer, "rte %02d  %s", iRte, sPrompt);
	strcpy (pShared->line1, buffer);

	// Format second line, start/end and total distance
	char sOrigin[8];
	char sDestination[8];
	char sRouteLength[10];
	SRteStored *pRteStored;
	if (iRte == 0) {
		pRteStored = &pData->rteActive.stored;
	} else {
		pRteStored = &pData->rteDatabase[iRte-1];
	}

	// Route endpoints
	if (pRteStored->nWaypoints < 2) {
		strcpy (sOrigin, "");
		strcpy (sDestination, "");
	} else {
		strcpy (sOrigin, pRteStored->waypoint[0].name);
		strcpy (sDestination, pRteStored->waypoint[pRteStored->nWaypoints-1].name);
	}

	// Format route length
	formatDistance (pRteStored->routeLength, pData->navUnits, sRouteLength);

	sprintf (pShared->line2, "%-5s/%-5s   %5s",
		sOrigin, sDestination, sRouteLength);

	// Format third line, parallel track or comment
	if (iRte == 0) {
		// Active route
		SRteActive *pRoute = &pData->rteActive;

		// Set left or right parallel track
		char cTrackLorR = pRoute->parallelTrackLeft ? 'L' : 'R';

		// Format parallel track distance
		char sParallelTrack[6];
		formatDistance (pRoute->parallelTrackDistance,
			pData->navUnits, sParallelTrack);

		sprintf (pShared->line3, "Parallel trk: %c%5s", cTrackLorR, sParallelTrack);
	} else {
		// Stored route
		SRteStored *pRoute = &pData->rteDatabase[iRte-1];

		// Line 3 is route comment
		strcpy (pShared->line3, pRoute->comment);
	}
}


void Route::UpdateRteCopyDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	char buffer[25];

	// Update line 1
	sprintf (buffer, "Copy to route %02d", copyToRte);
	strcpy (pShared->line1, buffer);

	// Update line 2
	sprintf (buffer, "  from route %02d?", copyFromRte);
	strcpy (pShared->line2, buffer);

	// Update line 3 with endpoints of from route
	char sOrigin[8];
	char sDestination[8];
	formatRteEndpoints (pData, copyFromRte, sOrigin, sDestination);
	sprintf (buffer, "         %-5s/%-5s", sOrigin, sDestination);
	strcpy (pShared->line3, buffer);
}



void Route::UpdateRteCpaDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1, header
	strcpy (pShared->line1, "Closest pt of apprch");

	// Line 2, route selection
	char sOrigin[8];
	char sDestination[8];
	formatRteEndpoints (pData, iRte, sOrigin, sDestination);
	sprintf (pShared->line2, "route %2d %-5s/%-5s", iRte, sOrigin, sDestination);

	// Line 3, CPA waypoint, brg/dist and confirmation
	char sId[8];
	strcpy (sId, textValue);
	// Blank out cursor if required
	if (!crsrFlash && (crsrMode == CrsrCpaId)) {
		sId[iText] = ' ';
	}

	char sBrg[8];
	formatBearing (cpaBrg, sBrg);

	char sDist[8];
	formatDistance (cpaDis, pData->navUnits, sDist);

	char sOk[8];
	if (crsrMode == CrsrCpaOk) {
		if (crsrFlash) {
			strcpy (sOk, "ok?");
		} else {
			strcpy (sOk, "   ");
		}
	} else {
		// Confirm mode not active, leave blank
		strcpy (sOk, "   ");
	}

	sprintf (pShared->line3, "%c:%s %s%s %s",
		CharFrom, sId, sBrg, sDist, sOk);
}


void Route::UpdateRteSearchDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	// Line 1, Route selection
	char sOrigin[8];
	char sDestination[8];
	formatRteEndpoints (pData, iRte, sOrigin, sDestination);
	sprintf (pShared->line1, "Route %2d %-5s/%-5s", iRte, sOrigin, sDestination);

	// Line 2, initial offset
	char sInitOffset[16];
	strcpy (sInitOffset, "");
	sprintf (pShared->line2, " initial offst:%s", sInitOffset);

	// Line 3, increment/confirm
	char sIncrement[16];
	strcpy (sIncrement, "");

	char sOk[8];
	if (crsrMode == CrsrSearchOk) {
		if (crsrFlash) {
			strcpy (sOk, "ok?");
		} else {
			strcpy (sOk, "   ");
		}
	} else {
		// Confirm mode not active, leave blank
		strcpy (sOk, "   ");
	}

	sprintf (pShared->line3, " increment:%s %s", sIncrement, sOk);
}


void Route::UpdateRteDisplay (void)
{
	switch (page) {
	case RteActive:
		UpdateRteActiveDisplay ();
		break;

	case RteCatalog:
		UpdateRteCatalogDisplay ();
		break;

	case RteCopy:
		UpdateRteCopyDisplay ();
		break;

	case RteSearch:
		UpdateRteSearchDisplay ();
		break;

	case RteCpa:
		UpdateRteCpaDisplay ();
		break;
	}
}



void Route::ReverseRte (int rte)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	// Activate the route
//	activateRteStored (&pData->rteActive,
//		               &pData->rteDatabase[rte-1],
//					   pRealTime->userPos,
//					   pData->autoLegSelect);

	// Reverse the active route
	reverseRteActive (pData);
}


//
// Update route 0 from the active Fly! flight plan
//
void Route::UpdateRteFromFlightPlan (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	SGps150RealTimeData *pRealTime = pData->pRealTime;

	// Get flight plan from Fly!, put into Route 0
	SFsimRoute *pRte = fsimGetActiveRoute ();

	// Activate the route
	activateRteStored (&pData->rteActive,
		               pRte,
					   pRealTime->userPos,
					   pData->autoLegSelect);

	// Activating the route results in the contents being copied; it is safe
	//   to free the route instantiated within fsimGetActiveRoute()
	fsimFreeRoute (pRte);
}


Route::Route (void *p) :
	FsmState (p), page(RteActive), crsr(false),
	iRte(0), iLeg(0), distMode(RteDistCum), timeMode(RteTimeEte),
	cpaWptValid(false)
{}


void Route::Activate (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	crsrMode = CrsrRteActivate;
	crsrFlash = true;

	// Set mode indicator light on
	pShared->liteRte = true;

	// Refresh RTE 0 with active Fly! flight plan if there is no active FPL
//	if (!pData->rteActive.valid) {
//		UpdateRte ();
//	}

	UpdateRteDisplay ();
}


void Route::Deactivate (void)
{
	// Set mode indicator light off
	pShared->liteRte = false;
}


void Route::HandleKey (char c)
{
	// Set the current char of the text entry field to the supplied character,
	//   and advance the cursor to the next char
	if (crsr) {
		textValue[iText] = c;
		iText++;
		if (iText > iTextBound) iText = iTextBound;
	}
}


bool Route::HandleTrigger (FsmTrigger t)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	bool rc = false;

	switch (t) {
	case TriggerCRSR:
		if (crsr) {
			// CRSR is active
			switch (page) {
			case RteCpa:
			case RteSearch:
				// Reset back to catalog page
				page = RteCatalog;
				crsrMode = CrsrRteActivate;
				break;

			default:
				crsr = false;
				crsrFlash = true;
			}
		} else {
			// CRSR is inactive, activate it
			crsr = true;
			crsrElapsed = 0;
			crsrFlash = true;
		}
		rc = true;
		break;

	case TriggerENT:
		switch (page) {
		case RteActive:
			// Do nothing
			break;

		case RteCatalog:
			if (crsr) {
				// CRSR is active
				switch (crsrMode) {
				case CrsrRteActivate:
					if (iRte == 0) {
						// Selected route is 0, transfer active flight plan from Fly
						UpdateRteFromFlightPlan ();
					} else {
						// Activate the stored route
//						activateRteStored (&pData->rteActive,
//							               &pData->rteDatabase[iRte-1],
//										   pRealTime->userPos,
//										   pData->autoLegSelect);
					}
					crsr = false;
					break;

				case CrsrRteReverse:
					// Activate the reversed selected route
					ReverseRte (iRte);
					crsr = false;
					break;

				case CrsrRteEdit:
					// Edit mode
					break;

				case CrsrRteDelete:
					// Delete route contents
					clearRteStored (pData->rteDatabase[iRte-1]);
					crsr = false;
					break;

				case CrsrRteCopy:
					// Enter copy mode if the current route is not route
					//   zero, and is empty
					if (iRte != 0) {
						// Transition to rte copy page only if empty
						if (pData->rteDatabase[iRte-1].nWaypoints == 0) {
							copyToRte = iRte;
							copyFromRte = 0;
							page = RteCopy;
						}
					}
					break;

				case CrsrRteCPA:
					page = RteCpa;
					crsrMode = CrsrCpaId;
					
					// Clear CPA fields
					cpaBrg = 0;
					cpaDis = 0;

					// Initialize text entry field for CPA waypoint
					iText = 0;
					iTextBound = 4;
					strcpy (textValue, "_____");
					break;

				case CrsrRteSearch:
					page = RteSearch;
					crsrMode = CrsrSearchOffset;
					break;
				}
			}
			break;

		case RteCopy:
			// User pressed ENT key in copy mode, perform copy
			copyRteStored (pData, copyFromRte, copyToRte);

			// Reset to catalog page, with TO route selected, no CRSR
			iRte = copyToRte;
			page = RteCatalog;
			crsr = false;
			break;

		case RteCpa:
			switch (crsrMode) {
			case CrsrCpaId:
				// Confirm parameters, search for waypoint
				if (searchWaypointId (pData, textValue, &cpaWpt)) {
					// CPA waypoint found; calculate CPA relative to the active route
					cpaWptValid = true;

					// Calculate CPA position relative to the selected route.
					//   Active leg is not needed
					int dummyLeg;
					calcCpaRoute (&pData->rteDatabase[iRte-1],
						          cpaWpt.pos,
								  &dummyLeg,
								  &cpaPos);

					// Update bearing and distance info
					calcBearingDistance (cpaWpt.pos, cpaPos, &cpaBrg, &cpaDis);
					cpaBrg -= pData->magVar;
				} else {
					// CPA waypoint not found
					cpaWptValid = false;
				}

				// Advance to confirmation
				crsrMode = CrsrCpaOk;
				break;

			case CrsrCpaOk:
				// Confirm CPA waypoint
				if (cpaWptValid) {
					// Create new user waypoint
					SUserWaypoint *pUser = &pData->userWpDatabase[pData->nUserWaypoints];

					// Formulate CPA user waypoint
					// TODO: Check suffix number to ensure uniqueness
					sprintf (pUser->name, "%s%1d", cpaWpt.id, 0);
					pUser->pos = cpaPos;
					pUser->rte = iRte;
					pData->nUserWaypoints++;

					// Insert new waypoint into selected route
					SRteWaypoint wpt;
					wpt.type = StoredWptUser;
					strcpy (wpt.name, cpaWpt.id);
					wpt.pos = cpaWpt.pos;
					insertRteStored (&pData->rteDatabase[iRte-1], &wpt, cpaFrom);
				}

				// Reset to catalog page
				page = RteCatalog;
				break;
			}
			break;

		case RteSearch:
			switch (crsrMode) {
			case CrsrSearchOffset:
				// Advance to increment
				crsrMode = CrsrSearchIncrement;
				break;

			case CrsrSearchIncrement:
				// Advance to confirmation
				crsrMode = CrsrSearchOk;
				break;

			case CrsrSearchOk:
				// Confirm parameters, update route
				break;
			}
			break;
		}
		rc = true;
		break;

	case TriggerCLR:
		switch (page) {
		case RteActive:
			// Clear CRSR if it is active
			crsr = false;
			break;

		case RteCatalog:
			// Clear CRSR if it is active
			crsr = false;
			break;

		case RteCopy:
			// User pressed CLR key in copy mode, abort copy
			// Reset to catalog page, with TO route selected, no CRSR
			iRte = copyToRte;
			page = RteCatalog;
			crsr = false;
			crsrFlash = true;
			break;
		}
		rc = true;
		break;
	
	case TriggerOuterRight:
		if (crsr) {
			// CRSR is active
			switch (page) {
			case RteActive:
				// Toggle between date and time cursor
				if (crsrMode == CrsrActiveTime) {
					crsrMode = CrsrActiveDistance;
				} else {
					crsrMode = CrsrActiveTime;
				}
				break;

			case RteCpa:
				// Increment edit index
				iText++;
				if (iText > iTextBound) iText = iTextBound;
				break;
			}
		} else {
			// CRSR is not active, toggle RTE page
			if (page == RteActive) {
				page = RteCatalog;
				crsrMode = CrsrRteActivate;
			} else {
				page = RteActive;
				crsrMode = CrsrActiveDistance;
			}
		}
		rc = true;
		break;

	case TriggerOuterLeft:
		if (crsr) {
			// CRSR is active
			switch (page) {
			case RteActive:
				// Toggle between date and time cursor
				if (crsrMode == CrsrActiveTime) {
					crsrMode = CrsrActiveDistance;
				} else {
					crsrMode = CrsrActiveTime;
				}
				break;

			case RteCpa:
				// Increment edit index
				if (iText > 0) iText--;
				break;
			}
		} else {
			// CRSR is not active, toggle RTE page
			if (page == RteActive) {
				page = RteCatalog;
				crsrMode = CrsrRteActivate;
			} else {
				page = RteActive;
				crsrMode = CrsrActiveDistance;
			}
		}
		rc = true;
		break;

	case TriggerInnerRight:
		if (crsr) {
			// CRSR is active
			switch (page) {
			case RteActive:
				// Increment date or time mode
				switch (crsrMode) {
				case CrsrActiveDistance:
					// Toggle distance mode between cum and leg
					if (distMode == RteDistCum) {
						distMode = RteDistLeg;
					} else {
						distMode = RteDistCum;
					}
					break;

				case CrsrActiveTime:
					// Cycle through time modes, ete eta and dtk
					switch (timeMode) {
					case RteTimeEte:
						timeMode = RteTimeEta;
						break;

					case RteTimeEta:
						timeMode = RteTimeDtk;
						break;

					case RteTimeDtk:
						timeMode = RteTimeEte;
						break;
					}
					break;
				}
				break;

			case RteCatalog:
				// Cycle through CRSR modes
				crsrMode++;
				if (crsrMode > CrsrRteSearch) crsrMode = CrsrRteActivate;
				break;

			case RteCopy:
				// Increment Copy From route
				copyFromRte++;
				if (copyFromRte > 19) copyFromRte = 0;
				break;

			case RteCpa:
				switch (crsrMode) {
				case CrsrCpaId:
					incEntry (textValue[iText]);
					break;
				}
				break;
			}
		} else {
			// CRSR is not active
			switch (page) {
			case RteActive:
				{
					// Increment active leg clamp to one less than number of legs
					//   in active route
					if (iLeg < (pData->rteActive.nLegs - 1)) iLeg++;
				}
				break;

			case RteCatalog:
				// Increment displayed route
				iRte++;
				if (iRte > 19) iRte = 0;
				break;
			}
		}
		rc = true;
		break;

	case TriggerInnerLeft:
		if (crsr) {
			// CRSR is active
			switch (page) {
			case RteActive:
				// Increment date or time mode
				switch (crsrMode) {
				case CrsrActiveDistance:
					// Toggle distance mode between cum and leg
					if (distMode == RteDistCum) {
						distMode = RteDistLeg;
					} else {
						distMode = RteDistCum;
					}
					break;

				case CrsrActiveTime:
					// Cycle through time modes, ete eta and dtk
					switch (timeMode) {
					case RteTimeEte:
						timeMode = RteTimeDtk;
						break;

					case RteTimeEta:
						timeMode = RteTimeEte;
						break;

					case RteTimeDtk:
						timeMode = RteTimeEta;
						break;
					}
					break;
				}
				break;

			case RteCatalog:
				// Cycle through CRSR modes
				crsrMode--;
				if (crsrMode < CrsrRteActivate) crsrMode = CrsrRteSearch;
				break;

			case RteCopy:
				// Decrement Copy From route
				copyFromRte--;
				if (copyFromRte < 0) copyFromRte = 19;
				break;

			case RteCpa:
				switch (crsrMode) {
				case CrsrCpaId:
					decEntry (textValue[iText]);
					break;
				}
				break;
			}
		} else {
			// CRSR is not active
			switch (page) {
			case RteActive:
				// Decrement active waypoint, clamp to zero
				if (iLeg > 0) iLeg--;
				break;

			case RteCatalog:
				// Decrement displayed route
				iRte--;
				if (iRte < 0) iRte = 19;
				break;
			}
		}
		rc = true;
		break;

	case TriggerKeyA:
		HandleKey ('A');
		break;

	case TriggerKeyB:
		HandleKey ('B');
		break;

	case TriggerKeyC:
		HandleKey ('C');
		break;

	case TriggerKeyD:
		HandleKey ('D');
		break;

	case TriggerKeyE:
		HandleKey ('E');
		break;

	case TriggerKeyF:
		HandleKey ('F');
		break;

	case TriggerKeyG:
		HandleKey ('G');
		break;

	case TriggerKeyH:
		HandleKey ('H');
		break;

	case TriggerKeyI:
		HandleKey ('I');
		break;

	case TriggerKeyJ:
		HandleKey ('J');
		break;

	case TriggerKeyK:
		HandleKey ('K');
		break;

	case TriggerKeyL:
		HandleKey ('L');
		break;

	case TriggerKeyM:
		HandleKey ('M');
		break;

	case TriggerKeyN:
		HandleKey ('N');
		break;

	case TriggerKeyO:
		HandleKey ('O');
		break;

	case TriggerKeyP:
		HandleKey ('P');
		break;

	case TriggerKeyQ:
		HandleKey ('Q');
		break;

	case TriggerKeyR:
		HandleKey ('R');
		break;

	case TriggerKeyS:
		HandleKey ('S');
		break;

	case TriggerKeyT:
		HandleKey ('T');
		break;

	case TriggerKeyU:
		HandleKey ('U');
		break;

	case TriggerKeyV:
		HandleKey ('V');
		break;

	case TriggerKeyW:
		HandleKey ('W');
		break;

	case TriggerKeyX:
		HandleKey ('X');
		break;

	case TriggerKeyY:
		HandleKey ('Y');
		break;

	case TriggerKeyZ:
		HandleKey ('Z');
		break;

	case TriggerKey1:
		HandleKey ('1');
		break;

	case TriggerKey2:
		HandleKey ('2');
		break;

	case TriggerKey3:
		HandleKey ('3');
		break;

	case TriggerKey4:
		HandleKey ('4');
		break;

	case TriggerKey5:
		HandleKey ('5');
		break;

	case TriggerKey6:
		HandleKey ('6');
		break;

	case TriggerKey7:
		HandleKey ('7');
		break;

	case TriggerKey8:
		HandleKey ('8');
		break;

	case TriggerKey9:
		HandleKey ('9');
		break;

	case TriggerKey0:
		HandleKey ('0');
		break;

	case TriggerKeySpace:
		HandleKey (' ');
		break;
	}
	UpdateRteDisplay ();

	return rc;
}


void Route::TimeSlice (float dT)
{
	static const float CRSR_ON_CYCLE  = (float)0.8;
	static const float CRSR_OFF_CYCLE = (float)0.2;

	// Increment cursor flash elapsed time
	if (crsr) {
		crsrElapsed += dT;
		if (crsrFlash) {
			// CRSR flash is on, delay for CRSR_ON_CYCLE time
			if (crsrElapsed > CRSR_ON_CYCLE) {
				crsrElapsed = 0;
				crsrFlash = false;
				UpdateRteDisplay ();
			}
		} else {
			// CRSR flash is off, delay for CRSR_OFF_CYCLE time
			if (crsrElapsed > CRSR_OFF_CYCLE) {
				crsrElapsed = 0;
				crsrFlash = true;
				UpdateRteDisplay ();
			}
		}
	}
}

//
// Set state
//
void Set::updateCDIArrivalDisplay (void)
{
	char buffer[24];

	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, page title
	strcpy (pShared->line1, "CDI/Arrival");

	// Line 2, CDI Scale
	char sCdiScale[16];
	strcpy (sCdiScale, " cdi scale");

	char sScale[16];
	if (crsr && !crsrFlash && (crsrMode==CDIScale)) {
		strcpy (sScale, "   ");
	} else {
		sprintf (sScale, "%3.2f", pData->cdiScale);
	}
	sprintf (buffer, "%s   %s", sCdiScale, sScale);
	strcpy (pShared->line2, buffer);

	// Line 3, Arrival alarm distance
	char sArrAlarm[16];
	strcpy (sArrAlarm, "arrival:");

	char sAlarm[16];
	if (crsr && !crsrFlash && (crsrMode==ArrivalAlarm)) {
		strcpy (sAlarm, "   ");
	} else {
		sprintf (sAlarm, "%3.1f", pData->arrivalAlarm);
	}

	sprintf (buffer, " %s  %s", sArrAlarm, sAlarm);
	strcpy (pShared->line3, buffer);
}

void Set::updateRouteSettingsDisplay (void)
{
	char buffer[24];
	
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1
	strcpy (pShared->line1, "Route Settings");

	// Set line 2, auto leg seq on/off
	sprintf (buffer, " auto leg seq  ");

	if (crsr && !crsrFlash && (crsrMode==AutoLegSeq)) {
		// Do nothing
	} else {
		if (pData->autoLegSeq)
			strcat (buffer, "on");
		else
			strcat (buffer, "off");
	}
	strcpy (pShared->line2, buffer);

	// Set line 2, auto leg select on/off
	sprintf (buffer, " auto leg slct ");
	if (crsr && !crsrFlash && (crsrMode==AutoLegSelect)) {
		// Do nothing
	} else {
		if (pData->autoLegSelect)
			strcat (buffer, "on");
		else 
			strcat (buffer, "off");
	}
	strcpy (pShared->line3, buffer);
}

void Set::updateMagVariationDisplay (void)
{
	char buffer[24];
	
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1
	strcpy (pShared->line1, "Magnetic Variation");

	// Set line 2, auto leg seq on/off
	sprintf (buffer, " ");
	char sMode[16];
	if (crsr && !crsrFlash && (crsrMode==MagVarMode)) {
		strcpy (sMode, "    ");
	} else {
		if (pData->magVarAuto)
			strcpy (sMode, "auto");
		else
			strcpy (sMode, "user");
	}

	char cMagVar;
	if (pData->magVar >= 0) {
		cMagVar = 'W';
	} else {
		cMagVar = 'E';
	}

	sprintf (pShared->line2, " %s mag: %c%03d",
		sMode, cMagVar, (int)pData->magVar);

	// Set line 3 blank
	strcpy (pShared->line3, "");
}

void Set::updateNrstApSearchDisplay (void)
{
	char buffer[24];
	
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1
	strcpy (pShared->line1, "Nearest apt search");

	// Set line 2, minimum runway length
	sprintf (buffer, "min rnwy len: %d", pData->minRwyLen);
	strcpy (pShared->line2, buffer);

	// Set line 3 runway surface
	if (crsr && !crsrFlash && (crsrMode==MinRwySetSurface)) {
		strcpy (buffer, "");
	} else {
		switch (pData->rwySurface) {
		case AnySurface:
			strcpy (buffer, "any srfc      ");
			break;

		case SoftHardSurface:
			strcpy (buffer, "soft/hard srfc");
			break;

		case HardSurface:
			strcpy (buffer, "hard only srfc");
			break;

		case WaterSurface:
			strcpy (buffer, "watr only srfc");
			break;
		}
	}
	strcpy (pShared->line3, buffer);
}

void Set::updateBatterySaverDisplay (void)
{
	char buffer[24];
	
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1
	strcpy (pShared->line1, "Battery saver--turn");

	// Set line 2, 
	sprintf (buffer, " off display %2d sec", pData->battSaver);
	strcpy (pShared->line2, buffer);

	// Set line 3 
	strcpy (pShared->line3, " after last keypress");
}

void Set::updateTripTimerDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1
	strcpy (pShared->line1, "Trip timer settings");

	// Set line 2, auto leg seq on/off
	char sMode[16];
	if (!crsrFlash && (crsrMode == TripTimerMode)) {
		strcpy (sMode, "");
	} else {
		if (pData->tripTimerPwrOn) {
			strcpy (sMode, "pwr is on");
		} else {
			strcpy (sMode, "gs exceeds");
		}
	}
	sprintf (pShared->line2, " run when %s", sMode);

	// Set line 3, threshold speed
	char sSpeed[16];
	if (pData->tripTimerPwrOn) {
		// Trip timer triggered by pwr on, don't display speed
		strcpy (sSpeed, "");
	} else {
		// Trip timer triggered by groundspeed
		formatSpeedInt (pData->tripTimerSpeed, pData->navUnits, sSpeed);

		// Blank out flashing cursor if necessary
		if (!crsrFlash) {
			switch (crsrMode) {
			case TripTimerSetSpeed1:
				sSpeed[0] = ' ';
				break;

			case TripTimerSetSpeed2:
				sSpeed[1] = ' ';
				break;

			case TripTimerSetSpeed3:
				sSpeed[2] = ' ';
				break;
			}
		}
	}

	sprintf (pShared->line3, "               %s", sSpeed);
}

void Set::updateMeasurementUnitsDisplay (void)
{
	char buffer[24];
	
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1, position/alt units
	char sPos[4];
	if (crsr && !crsrFlash && (crsrMode==UnitsPosition)) {
		strcpy (sPos, "   ");
	} else {
		switch (pData->posUnits) {
		case PosDms:
			strcpy (sPos, "dms");
			break;

		case PosDm:
			strcpy (sPos, "dm");
		}
	}

	char cAlt;
	if (crsr && !crsrFlash && (crsrMode==UnitsAlt)) {
		cAlt = ' ';
	} else {
		switch (pData->altUnits) {
		case AltFeet:
			cAlt = CharFeet;
			break;

		case AltMetres:
			cAlt = CharMetres;
			break;
		}
	}

	char sVs[4];
	if (crsr && !crsrFlash && (crsrMode==UnitsVs)) {
		strcpy (sVs, "   ");
	} else {
		switch (pData->vsUnits) {
		case VsFpm:
			strcpy (sVs, "fpm");
			break;

		case VsMpm:
			strcpy (sVs, "mpm");
			break;

		case VsMps:
			strcpy (sVs, "mps");
			break;
		}
	}

	sprintf (buffer, "posn %-3s alt  %c %-3s",
		sPos, cAlt, sVs);
	strcpy (pShared->line1, buffer);

	// Set line 2, nav/fuel units
	char cDist = '?';
	char cSpeed = '?';
	if (crsr && !crsrFlash && (crsrMode==UnitsNav)) {
		cDist = ' ';
		cSpeed = ' ';
	} else {
		switch (pData->navUnits) {
		case NavNmKt:
			cDist = CharNm;
			cSpeed = CharKts;
			break;

		case NavMiMph:
			cDist = CharMi;
			cSpeed = CharMph;
			break;

		case NavKmKph:
			cDist = CharKm;
			cSpeed = CharKph;
			break;
		}
	}

	char cFuel;
	if (crsr && !crsrFlash && (crsrMode==UnitsFuel)) {
		cFuel = ' ';
	} else {
		switch (pData->fuelUnits) {
		case FuelUSGal:
			cFuel = CharUSGal;
			break;

		case FuelImpGal:
			cFuel = CharImpGal;
			break;

		case FuelKg:
			cFuel = CharKg;
			break;

		case FuelLbs:
			cFuel = CharLbs;
			break;

		case FuelLitres:
			cFuel = CharLitres;
			break;
		}
	}

	char sFuelType[5];
	if (crsr && !crsrFlash && (crsrMode==UnitsFuelType)) {
		strcpy (sFuelType, "    ");
	} else {
		switch (pData->fuelTypeUnits) {
		case FuelAvGas:
			strcpy (sFuelType, "avgs");
			break;

		case FuelJetA:
			strcpy (sFuelType, "jetA");
			break;

		case FuelJetB:
			strcpy (sFuelType, "jetB");
			break;
		}
	}

	sprintf (buffer, "nav  %c %c fuel %c %-4s",
		cDist, cSpeed, cFuel, sFuelType);
	strcpy (pShared->line2, buffer);

	// Set line 3 pressure/temp units
	char cPress;
	if (crsr && !crsrFlash && (crsrMode==UnitsPressure)) {
		cPress = ' ';
	} else {
		switch (pData->pressureUnits) {
		case PressHg:
			cPress = CharHg;
			break;

		case PressMb:
			cPress = CharHg;
			break;
		}
	}

	char cTemp;
	if (crsr && !crsrFlash && (crsrMode==UnitsTemperature)) {
		cTemp = ' ';
	} else {
		switch (pData->tempUnits) {
		case TempF:
			cTemp = CharDegF;
			break;

		case TempC:
			cTemp = CharDegC;
			break;
		}
	}

	sprintf (buffer, "pres %c   temp %c", cPress, cTemp);
	strcpy (pShared->line3, buffer);
}

void Set::updateSUASettingsDisplay (void)
{
	char buffer[24];
	
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1
	strcpy (buffer, "SUA Settings");
	strcpy (pShared->line1, buffer);

	// Set line 2
	strcpy (buffer, "");
	strcpy (pShared->line2, buffer);

	// Set line 3
	strcpy (buffer, "Not implemented");
	strcpy (pShared->line3, buffer);
}

void Set::updateMapDatumDisplay (void)
{
	char buffer[24];
	
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1
	strcpy (buffer, "Map Datum");
	strcpy (pShared->line1, buffer);

	// Set line 2
	strcpy (buffer, "");
	strcpy (pShared->line2, buffer);

	// Set line 3
	strcpy (buffer, "Not implemented");
	strcpy (pShared->line3, buffer);
}

void Set::updateIOChannelsDisplay (void)
{
	char buffer[24];
	
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1
	strcpy (buffer, "IO Channels");
	strcpy (pShared->line1, buffer);

	// Set line 2
	strcpy (buffer, "");
	strcpy (pShared->line2, buffer);

	// Set line 3
	strcpy (buffer, "Not implemented");
	strcpy (pShared->line3, buffer);
}

void Set::updateARINC429Display (void)
{
	char buffer[24];
	
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1
	strcpy (buffer, "ARINC429");
	strcpy (pShared->line1, buffer);

	// Set line 2
	strcpy (buffer, "");
	strcpy (pShared->line2, buffer);

	// Set line 3
	strcpy (buffer, "Not implemented");
	strcpy (pShared->line3, buffer);
}

void Set::updateDataTransferDisplay (void)
{
	char buffer[24];
	
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set line 1
	strcpy (buffer, "Data Transfer");
	strcpy (pShared->line1, buffer);

	// Set line 2
	strcpy (buffer, "");
	strcpy (pShared->line2, buffer);

	// Set line 3
	strcpy (buffer, "Not implemented");
	strcpy (pShared->line3, buffer);
}


void Set::updateDisplay (void)
{
	switch (page) {
	case CDIArrival:
		updateCDIArrivalDisplay ();
		break;

	case RouteSettings:
		updateRouteSettingsDisplay ();
		break;

	case MagVariation:
		updateMagVariationDisplay ();
		break;

	case NrstApSearch:
		updateNrstApSearchDisplay ();
		break;

	case BatterySaver:
		updateBatterySaverDisplay ();
		break;

	case TripTimer:
		updateTripTimerDisplay ();
		break;

	case MeasurementUnits:
		updateMeasurementUnitsDisplay ();
		break;

	case SUASettings:
		updateSUASettingsDisplay ();
		break;

	case MapDatum:
		updateMapDatumDisplay ();
		break;

	case IOChannels:
		updateIOChannelsDisplay ();
		break;

	case ARINC429:
		updateARINC429Display ();
		break;

	case DataTransfer:
		updateDataTransferDisplay ();
		break;
	}
}

void Set::Activate (void)
{
	crsr = false;

	// Set mode indicator light on
	pShared->liteSet = true;

	updateDisplay ();
}


void Set::Deactivate (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Set mode indicator light off
	pShared->liteSet = false;

	// Save INI settings to file
	SaveGPSConfiguration (pData);
}


static void incPage (ESetPage &page)
{
	switch (page) {
	case CDIArrival:
		page = RouteSettings;
		break;

	case RouteSettings:
		page = MagVariation;
		break;

	case MagVariation:
		page = NrstApSearch;
		break;

	case NrstApSearch:
		page = BatterySaver;
		break;

	case BatterySaver:
		page = TripTimer;
		break;

	case TripTimer:
		page = MeasurementUnits;
		break;

	case MeasurementUnits:
		page = SUASettings;
		break;

	case SUASettings:
		page = MapDatum;
		break;

	case MapDatum:
		page = IOChannels;
		break;

	case IOChannels:
		page = ARINC429;
		break;

	case ARINC429:
		page = DataTransfer;
		break;

	case DataTransfer:
		page = CDIArrival;
		break;
	}
}


static void decPage (ESetPage &page)
{
	switch (page) {
	case CDIArrival:
		page = DataTransfer;
		break;

	case RouteSettings:
		page = CDIArrival;
		break;

	case MagVariation:
		page = RouteSettings;
		break;

	case NrstApSearch:
		page = MagVariation;
		break;

	case BatterySaver:
		page = NrstApSearch;
		break;

	case TripTimer:
		page = BatterySaver;
		break;

	case MeasurementUnits:
		page = TripTimer;
		break;

	case SUASettings:
		page = MeasurementUnits;
		break;

	case MapDatum:
		page = SUASettings;
		break;

	case IOChannels:
		page = MapDatum;
		break;

	case ARINC429:
		page = IOChannels;
		break;

	case DataTransfer:
		page = ARINC429;
		break;

	}
}


bool Set::HandleTrigger (FsmTrigger t)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	bool rc = false;

	switch (t) {
	case TriggerCRSR:
		crsr = !crsr;
		crsrElapsed = 0;
		crsrFlash = true;
		switch (page) {
		case CDIArrival:
			crsrMode = CDIScale;
			break;

		case RouteSettings:
			crsrMode = AutoLegSeq;
			break;

		case MagVariation:
			crsrMode = MagVarMode;
			break;

		case NrstApSearch:
			crsrMode = MinRwyLengthSet1;
			break;

		case BatterySaver:
			crsrMode = BattSaverInterval;
			break;

		case TripTimer:
			crsrMode = TripTimerMode;
			break;

		case MeasurementUnits:
			crsrMode = UnitsPosition;
			break;

		case SUASettings:
		case MapDatum:
		case IOChannels:
		case ARINC429:
		case DataTransfer:
			break;
		}
		rc = true;
		break;

	case TriggerOuterRight:
		if (crsr) {
			switch (page) {
			case CDIArrival:
				if (crsrMode == CDIScale)
					crsrMode = ArrivalAlarm;
				else
					crsrMode = CDIScale;
				break;

			case RouteSettings:
				if (crsrMode == AutoLegSeq)
					crsrMode = AutoLegSelect;
				else
					crsrMode = AutoLegSeq;
				break;

			case MagVariation:
				if (!pData->magVarAuto) {
					switch (crsrMode) {
					case MagVarMode:
						crsrMode = MagVarSet1;
						break;

					case MagVarSet1:
						crsrMode = MagVarSet2;
						break;

					case MagVarSet2:
						crsrMode = MagVarSet3;
						break;

					case MagVarSet3:
						crsrMode = MagVarSet4;
						break;

					case MagVarSet4:
						crsrMode = MagVarMode;
						break;
					}
				}
				break;

			case NrstApSearch:
				switch (crsrMode) {
				case MinRwyLengthSet1:
					crsrMode = MinRwyLengthSet2;
					break;

				case MinRwyLengthSet2:
					crsrMode = MinRwyLengthSet3;
					break;

				case MinRwyLengthSet3:
					crsrMode = MinRwyLengthSet4;
					break;

				case MinRwyLengthSet4:
					crsrMode = MinRwySetSurface;
					break;
				
				case MinRwySetSurface:
					crsrMode = MinRwyLengthSet1;
					break;
				}
				break;

			case BatterySaver:
				break;

			case TripTimer:
				switch (crsrMode) {
				case TripTimerMode:
					crsrMode = TripTimerSetSpeed1;
					break;

				case TripTimerSetSpeed1:
					crsrMode = TripTimerSetSpeed2;
					break;

				case TripTimerSetSpeed2:
					crsrMode = TripTimerSetSpeed3;
					break;

				case TripTimerSetSpeed3:
					crsrMode = TripTimerMode;
					break;
				}
				break;

			case MeasurementUnits:
				switch (crsrMode) {
				case UnitsPosition:
					crsrMode = UnitsAlt;
					break;

				case UnitsAlt:
					crsrMode = UnitsVs;
					break;

				case UnitsVs:
					crsrMode = UnitsNav;
					break;

				case UnitsNav:
					crsrMode = UnitsFuel;
					break;

				case UnitsFuel:
					crsrMode = UnitsFuelType;
					break;

				case UnitsFuelType:
					crsrMode = UnitsPressure;
					break;

				case UnitsPressure:
					crsrMode = UnitsTemperature;
					break;

				case UnitsTemperature:
					crsrMode = UnitsPosition;
					break;
				}

				break;

			case SUASettings:
			case MapDatum:
			case IOChannels:
			case ARINC429:
			case DataTransfer:
				break;
			}
		} else {
			// Increment SET page
			incPage (page);
		}
		rc = true;
		break;

	case TriggerOuterLeft:
		if (crsr) {
			switch (page) {
			case CDIArrival:
				if (crsrMode == CDIScale)
					crsrMode = ArrivalAlarm;
				else
					crsrMode = CDIScale;
				break;

			case RouteSettings:
				if (crsrMode == AutoLegSeq)
					crsrMode = AutoLegSelect;
				else
					crsrMode = AutoLegSeq;
				break;

			case MagVariation:
				if (!pData->magVarAuto) {
					switch (crsrMode) {
					case MagVarMode:
						crsrMode = MagVarSet4;
						break;

					case MagVarSet1:
						crsrMode = MagVarMode;
						break;

					case MagVarSet2:
						crsrMode = MagVarSet1;
						break;

					case MagVarSet3:
						crsrMode = MagVarSet2;
						break;

					case MagVarSet4:
						crsrMode = MagVarSet3;
						break;
					}
				}
				break;

			case NrstApSearch:
				switch (crsrMode) {
				case MinRwyLengthSet1:
					crsrMode = MinRwySetSurface;
					break;

				case MinRwyLengthSet2:
					crsrMode = MinRwyLengthSet1;
					break;

				case MinRwyLengthSet3:
					crsrMode = MinRwyLengthSet2;
					break;

				case MinRwyLengthSet4:
					crsrMode = MinRwyLengthSet3;
					break;
				
				case MinRwySetSurface:
					crsrMode = MinRwyLengthSet4;
					break;
				}
				break;

			case BatterySaver:
				break;

			case TripTimer:
				switch (crsrMode) {
				case TripTimerMode:
					crsrMode = TripTimerSetSpeed3;
					break;

				case TripTimerSetSpeed1:
					crsrMode = TripTimerMode;
					break;

				case TripTimerSetSpeed2:
					crsrMode = TripTimerSetSpeed1;
					break;

				case TripTimerSetSpeed3:
					crsrMode = TripTimerSetSpeed2;
					break;
				}
				break;

			case MeasurementUnits:
				switch (crsrMode) {
				case UnitsPosition:
					crsrMode = UnitsTemperature;
					break;

				case UnitsAlt:
					crsrMode = UnitsPosition;
					break;

				case UnitsVs:
					crsrMode = UnitsAlt;
					break;

				case UnitsNav:
					crsrMode = UnitsVs;
					break;

				case UnitsFuel:
					crsrMode = UnitsNav;
					break;

				case UnitsFuelType:
					crsrMode = UnitsFuel;
					break;

				case UnitsPressure:
					crsrMode = UnitsFuelType;
					break;

				case UnitsTemperature:
					crsrMode = UnitsPressure;
					break;
				}
				break;

			case SUASettings:
			case MapDatum:
			case IOChannels:
			case ARINC429:
			case DataTransfer:
				break;
			}
		} else {
			// Decrement SET page
			decPage (page);
		}
		rc = true;
		break;

	case TriggerInnerRight:
		if (crsr) {
			switch (crsrMode) {
			case CDIScale:
				toggleCdiScale (pData->cdiScale);
				break;

			case ArrivalAlarm:
				break;

			case AutoLegSeq:
				pData->autoLegSeq = !pData->autoLegSeq;
				break;

			case AutoLegSelect:
				pData->autoLegSelect = !pData->autoLegSelect;
				break;

			case MagVarSet1:
				// Toggle E/W
				pData->magVar = -pData->magVar;
				break;

			case MagVarSet2:
				incHundreds (pData->magVar);
				break;

			case MagVarSet3:
				incTens (pData->magVar);
				break;

			case MagVarSet4:
				incOnes (pData->magVar);
				break;

			case MagVarMode:
				pData->magVarAuto = !pData->magVarAuto;
				break;

			case MinRwyLengthSet1:
				incThousands (pData->minRwyLen);
				break;

			case MinRwyLengthSet2:
				incHundreds (pData->minRwyLen);
				break;

			case MinRwyLengthSet3:
				incTens (pData->minRwyLen);
				break;

			case MinRwyLengthSet4:
				incOnes (pData->minRwyLen);
				break;
			
			case MinRwySetSurface:
				incRwySurface (pData->rwySurface);
				break;

			case TripTimerMode:
				// Toggle trip timer mode
				pData->tripTimerPwrOn = !pData->tripTimerPwrOn;
				break;

			case TripTimerSetSpeed1:
				incHundreds (pData->tripTimerSpeed);
				break;

			case TripTimerSetSpeed2:
				incTens (pData->tripTimerSpeed);
				break;

			case TripTimerSetSpeed3:
				incOnes (pData->tripTimerSpeed);
				break;

			case UnitsPosition:
				incPosUnits (pData->posUnits);
				break;

			case UnitsAlt:
				incAltUnits (pData->altUnits);
				break;

			case UnitsVs:
				incVsUnits (pData->vsUnits);
				break;

			case UnitsNav:
				incNavUnits (pData->navUnits);
				break;

			case UnitsFuel:
				incFuelUnits (pData->fuelUnits);
				break;

			case UnitsFuelType:
				incFuelTypeUnits (pData->fuelTypeUnits);
				break;

			case UnitsPressure:
				incPressureUnits (pData->pressureUnits);
				break;

			case UnitsTemperature:
				incTempUnits (pData->tempUnits);
				break;
			}
		}
		rc = true;
		break;

	case TriggerInnerLeft:
		if (crsr) {
			switch (crsrMode) {
			case CDIScale:
				toggleCdiScale (pData->cdiScale);
				break;

			case ArrivalAlarm:
				break;

			case AutoLegSeq:
				pData->autoLegSeq = !pData->autoLegSeq;
				break;

			case AutoLegSelect:
				pData->autoLegSelect = !pData->autoLegSelect;
				break;

			case MagVarSet1:
				pData->magVar = -pData->magVar;
				break;

			case MagVarSet2:
				decHundreds (pData->magVar);
				break;

			case MagVarSet3:
				decTens (pData->magVar);
				break;

			case MagVarSet4:
				decOnes (pData->magVar);
				break;

			case MagVarMode:
				pData->magVarAuto = !pData->magVarAuto;
				break;

			case MinRwyLengthSet1:
				decThousands (pData->minRwyLen);
				break;

			case MinRwyLengthSet2:
				decHundreds (pData->minRwyLen);
				break;

			case MinRwyLengthSet3:
				decTens (pData->minRwyLen);
				break;

			case MinRwyLengthSet4:
				decOnes (pData->minRwyLen);
				break;
			
			case MinRwySetSurface:
				decRwySurface (pData->rwySurface);
				break;

			case TripTimerMode:
				// Toggle trip timer mode
				pData->tripTimerPwrOn = !pData->tripTimerPwrOn;
				break;

			case TripTimerSetSpeed1:
				decHundreds (pData->tripTimerSpeed);
				break;

			case TripTimerSetSpeed2:
				decTens (pData->tripTimerSpeed);
				break;

			case TripTimerSetSpeed3:
				decOnes (pData->tripTimerSpeed);
				break;

			case UnitsPosition:
				decPosUnits (pData->posUnits);
				break;

			case UnitsAlt:
				decAltUnits (pData->altUnits);
				break;

			case UnitsVs:
				decVsUnits (pData->vsUnits);
				break;

			case UnitsNav:
				decNavUnits (pData->navUnits);
				break;

			case UnitsFuel:
				decFuelUnits (pData->fuelUnits);
				break;

			case UnitsFuelType:
				decFuelTypeUnits (pData->fuelTypeUnits);
				break;

			case UnitsPressure:
				decPressureUnits (pData->pressureUnits);
				break;

			case UnitsTemperature:
				decTempUnits (pData->tempUnits);
				break;
			}
		}
		rc = true;
		break;
	}

	updateDisplay ();

	return rc;
}


void Set::TimeSlice (float dT)
{
	static const float UpdateInterval = (float)1.0;
	static const float CrsrOnInterval = (float)0.8;
	static const float CrsrOffInterval = (float)0.2;
	static float elapsed = 0;

	// Increment elapsed time since last real-time update
	elapsed += dT;
	if (elapsed > UpdateInterval) {
		elapsed = 0;
		updateDisplay ();
	}
	
	// Increment cursor elapsed time
	if (crsr) {
		crsrElapsed += dT;
		if (crsrFlash && (crsrElapsed > CrsrOnInterval)) {
			crsrFlash = false;
			crsrElapsed = 0;
			updateDisplay ();
		} else if (!crsrFlash && (crsrElapsed > CrsrOffInterval)) {
			crsrFlash = true;
			crsrElapsed = 0;
			updateDisplay ();
		}
	}
}

//
// Status state
//
static const char *statusMsg[] =
{
	"Altitude Input Fail",
	"Apprchng offset %s",
	"Approaching %s",
	"Arrival at %s",
	"Arrvl at offst %s",
	"Battery low",
	"Battery rqrs service",
	"Cannot chg actv wpt",
	"Cannot nav lckd rte",
	"Cannot ofst goto rte",
	"Checklist is full",
	"Collecting data",
	"Comment memory full",
	"Data card failed",
	"Data card write failed",
	"Degraded accuracy",
	"Do not use for nav",
	"Final altitude alert",
	"Fuel/Air input fail",
	"Inside SUA",
	"Invalid CPA wpt %s",
	"Invalid copy route",
	"Invalid CPA route",
	"Invalid SAR route",
	"Leg not smoothed",
	"Memory battery low",
	"Near SUA < 2nm",
	"Need Alt - press NAV",
	"No altitude input",
	"Offset nav cancelled",
	"Offset nav in effect",
	"Ofst too big for rte",
	"Osc needs adjustment",
	"Poor GPS coverage",
	"Prox alarm - press NAV",
	"Proximity overlap",
	"Proximity wpt locked",
	"Proximity wpt moved",
	"Proximity wpt deleted",
	"Pwr down and re-init",
	"RAM failed",
	"Ready for navigation",
	"Received invalid wpt",
	"Receiver failed",
	"ROM failed",
	"Route is empty",
	"Route is full",
	"Rte wpt deleted",
	"Rte wpt locked",
	"Rte wpt moved",
	"Searching the sky",
	"Start altitude chnge",
	"Steep turn ahead",
	"Stored data lost",
	"SUA ahead < 10 min",
	"SUA near & ahead",
	"Timer expired",
	"User data RX started",
	"User data Tx complete",
	"VNAV cancelled",
	"Wpt comment locked",
	"Wpt exists %s",
	"Wpt memory full"
};


static const char *receiverStatusMsg[] =
	{
		"Search Sky",
		"Acquiring",
		"2D Nav",
		"3D Nav",
		"Simulator",
		"Poor Cvrge",
		"Need Alt",
		"Not Usable"
	};


//
// Update Satellite Receiver page of STAT mode
//
void Status::UpdateStatusReceiverDisplay (void)
{
	char sEpeDop[8];
	char sValue[8];
	if (precisionMode == PrecisionEpe) {
		strcpy (sEpeDop, "epe");
		formatEpe(precision, sValue);
	} else {
		strcpy (sEpeDop, "dop");
		formatDop (precision, sValue);
	}

	// Line 1, receiver status and dop/ete
	sprintf (pShared->line1, "%10s   %3s %2s",
		receiverStatusMsg[rcvrStatus], sEpeDop, sValue);

	char sAcquired[24];
	char sSignal[24];
	strcpy (sAcquired, "sat ");
	strcpy (sSignal, "sgl ");
	for (int i=0; i<nSatellites; i++) {
		char sValue[8];
		formatSatellite (satAcquired[i], sValue);
		strcat (sAcquired, sValue);

		formatSignal (satSignal[i], sValue);
		strcat (sSignal, sValue);
	}

	// Line 2, acquired satellites
	strcpy (pShared->line2, sAcquired);

	// Line 3, signal strengths
	strcpy (pShared->line3, sSignal);
}


//
// Update Message page of STAT mode
//
void Status::UpdateStatusMsgDisplay (void)
{

}


//
//
//
void Status::UpdateStatusDisplay ()
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	switch (page) {
		case StatusReceiver:
			UpdateStatusReceiverDisplay ();
			break;

		case StatusMsg:
			UpdateStatusMsgDisplay ();
			break;

	}
}

void Status::Activate (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Check if a status message is pending
	if (pData->msgPending) {
		// Initialize to Msg page
		page = StatusMsg;
		pData->msgPending = false;
	} else {
		// Initialize to Satellite Receiver page
		page = StatusReceiver;
	}

	// Put dummy data into received satellite fields
	nSatellites = 8;
	srand((unsigned int)time(NULL));
	for (int i=0; i<nSatellites; i++) {
		satAcquired[i] = (rand() % 24) + 1;
		satSignal[i] = (rand() % 10);
	}
	rcvrStatus = Simulator;

	UpdateStatusDisplay ();
}


void Status::Deactivate (void)
{
}


bool Status::HandleTrigger (FsmTrigger t)
{
	bool rc = false;

	switch (t) {
		case TriggerOuterRight:
			break;

		case TriggerOuterLeft:
			break;
	}

	return rc;
}


void Status::TimeSlice (float dT)
{
	// Receiver mode is determined by the number of satellites
	//   which are visible
	//   0  = Searching Sky
	//   1  = Acquiring
	//   2  = 2D Nav
	//   3+ = 3D Nav
	// Various error and fault modes are not currently supported
}

//
// Waypoint state
//
static const int CrsrWptMenuAprt = 0;
static const int CrsrWptMenuVor = 1;
static const int CrsrWptMenuNdb = 2;
static const int CrsrWptMenuInt = 3;
static const int CrsrWptMenuUser = 4;

typedef enum {
	// Non-edit cursor modes
	CrsrAptId,
	CrsrAptName,
	CrsrAptCommentLine1,
	CrsrAptCommentLine2,
	CrsrVorId,
	CrsrVorName,
	CrsrVorCommentLine1,
	CrsrVorCommentLine2,
	CrsrNdbId,
	CrsrNdbName,
	CrsrNdbCommentLine1,
	CrsrNdbCommentLine2,
	CrsrFixId,
	CrsrUserId,

	// Edit cursor modes
	CrsrAptIdEdit,
	CrsrAptNameEdit,
	CrsrAptCommentLine1Edit,
	CrsrAptCommentLine2Edit,
	CrsrVorIdEdit,
	CrsrVorCommentLine1Edit,
	CrsrVorCommentLine2Edit,
	CrsrNdbIdEdit,
	CrsrNdbNameEdit,
	CrsrNdbCommentLine1Edit,
	CrsrNdbCommentLine2Edit,
	CrsrFixIdEdit,
	CrsrUserIdEdit,

	CrsrUserNewPosn,
	CrsrUserNewRefWpt,
	CrsrUserNewRngBrg,
} ECrsrMode;


static void incWptMenuCrsr (int &i)
{
	i++; if (i>CrsrWptMenuUser) i = CrsrWptMenuAprt;
}

static void decWptMenuCrsr (int &i)
{
	i--; if (i<CrsrWptMenuAprt) i = CrsrWptMenuUser;
}


//
// Formatting function for proximity waypoint item
//
void Waypoint::FormatProximityWaypoint (int iWpt, char *buffer)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	SProximityWaypoint *pWp = &pData->proxWpDatabase[iWpt];

	char sName[8];
	char sDist[8];
	if (strlen(pWp->name) == 0) {
		strcpy (sName, "_____");
		formatEmptyDistance (pData->navUnits, sDist);
	} else {
		strcpy (sName, pWp->name);
		formatDistance (pWp->distance, pData->navUnits, sDist);
	}

	sprintf (buffer, "%2d:%5s dis:%4s", iWpt+1, sName, sDist);
}


//
// Helper function to increment the proximity waypoint index
//
static int incProxWaypoint (int iWpt)
{
	iWpt++;
	if (iWpt >= 9) iWpt = 0;
	return iWpt;
}


//
// Helper function to decrement the proximity waypoint index
//
static int decProxWaypoint (int iWpt)
{
	iWpt--;
	if (iWpt < 0) iWpt = 8;
	return iWpt;
}


//
// Increment the runway index for the Airport Runway page.  This function is called
//  in response to the InnerRight trigger when the user is on this page.  As the
//  knob is turned, all approach data for the given runway is scrolled, then when
//  all approach data has been displayed, the next runway is displayed
//
void Waypoint::IncAptRwy ()
{
	if (wptAirport.valid) {
		// First check if approach index can be decremented
		int nApproach = wptAirport.rwy[iAptRwy].nApproach;
		if (iAptRwyApproach < (nApproach-1)) {
			// Increment approach index
			iAptRwyApproach++;
		} else {
			// Check if the runway index can be incremented
			if (iAptRwy < wptAirport.nRunways-1) {
				// Increment runway index
				iAptRwy++;
				iAptRwyApproach = 0;
			}
		}
	}
}

void Waypoint::DecAptRwy ()
{
	if (wptAirport.valid) {
		// First check if approach index can be decremented
		if (iAptRwyApproach > 0) {
			// Decrement approach index
			iAptRwyApproach--;
		} else {
			// Check if the runway index can be incremented
			if (iAptRwy > 0) {
				// Increment runway index
				iAptRwy--;
				int nApproach = wptAirport.rwy[iAptRwy].nApproach;
				if (nApproach == 0) {
					// No approaches
					iAptRwyApproach = 0;
				} else {
					// Initialize approach index to last one
					iAptRwyApproach = nApproach-1;
				}
			}
		}
	}
}


void Waypoint::CopyWptAirport (SAirport       *pAirport,
							   SWptAirport    *pWpt)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	formatAirportId (pAirport, pWpt->id);
	strcpy (pWpt->name, pAirport->name);
	strcpy (pWpt->city, pAirport->city);
	pWpt->pos = pAirport->pos;
	pWpt->usage = (EAirportUsage)pAirport->usage;

	// Copy runway data
	SRunway *pNextRwy = pAirport->runways;
	int nRwys = 0;
	SWptRunway *pRwy;
	while (pNextRwy && (nRwys < 16)) {

		pRwy = &pWpt->rwy[nRwys];
		pRwy->length = pNextRwy->length;
		sprintf (pRwy->id, "%3s/%-3s", pNextRwy->recip.id, pNextRwy->base.id);
		pRwy->surface = (ERwySurface)pNextRwy->surfaceType;

		// Copy ILS and/or localizer data
		pRwy->nApproach = 0;
		SILS *pLoc = pNextRwy->recip.localizer;
		SILS *pGS = pNextRwy->recip.glideslope;
		if (pLoc) {
			// Localizer exists
			SWptRwyApproach &a = pRwy->approach[pRwy->nApproach];

			if (pGS) {
				// Glideslope exists too, this is full ILS
				a.type = WptRwyApprIls;
			} else {
				// Localizer but no glideslope
				a.type = WptRwyApprLoc;
			}
			strcpy (a.rwyId, pLoc->runwayID);
			a.freq = pLoc->freq;
			pRwy->nApproach++;
		}

		pLoc = pNextRwy->base.localizer;
		pGS = pNextRwy->base.glideslope;
		if (pLoc) {
			// Localizer exists
			SWptRwyApproach &a = pRwy->approach[pRwy->nApproach];

			if (pGS) {
				// Glideslope exists too, this is full ILS
				a.type = WptRwyApprIls;
			} else {
				// Localizer but no glideslope
				a.type = WptRwyApprLoc;
			}
			strcpy (a.rwyId, pLoc->runwayID);
			a.freq = pLoc->freq;
			pRwy->nApproach++;
		}

		nRwys++;
		pNextRwy = pNextRwy->next;
	}
	pWpt->nRunways = nRwys;
	pWpt->iRunway = 0;

	// Copy comms data
	SComm *pNextComm = pAirport->comms;
	int nComms = 0;
	SWptComm *pComm;
	while (pNextComm && (nComms < 16)) {

		pComm = &pWpt->comm[nComms];
		pComm->freq = pNextComm->freq[0];
		formatCommType (pNextComm->type, pComm->name);

		nComms++;
		pNextComm = pNextComm->next;
	}
	pWpt->nComms = nComms;
	pWpt->iComm = 0;

	// Search for a waypoint comment for this airport
	int i = searchWptComment (pData, StoredWptAirport, pWpt->id);
	if (i != -1) {
		memcpy (&pWpt->comment, &pData->wptCommentDatabase[i], sizeof(SWptComment));
	} else {
		// No comments found, initialize with defaults
		pWpt->comment.type = StoredWptAirport;
		strcpy (pWpt->comment.id, pWpt->id);
		strcpy (pWpt->comment.line1, "____________________");
		strcpy (pWpt->comment.line2, "____________________");
	}
}

void Waypoint::CopyWptNdb (SNavaid  *pNdb,
						   SWptNdb  *pWpt)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	strcpy (pWpt->id, pNdb->id);
	strcpy (pWpt->name, pNdb->name);
	pWpt->freq = pNdb->freq;
	pWpt->pos = pNdb->pos;

	// City and region unknown
	memset (pWpt->city, 0, 40);

	// Search for a waypoint comment for this NDB
	int i = searchWptComment (pData, StoredWptNdb, pWpt->id);
	if (i != -1) {
		memcpy (&pWpt->comment, &pData->wptCommentDatabase[i], sizeof(SWptComment));
	} else {
		// No comments found, initialize with defaults
		pWpt->comment.type = StoredWptNdb;
		strcpy (pWpt->comment.id, pWpt->id);
		strcpy (pWpt->comment.line1, "____________________");
		strcpy (pWpt->comment.line2, "____________________");
	}
}

void Waypoint::CopyWptVor (SNavaid  *pVor,
						   SWptVor  *pWpt)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	strcpy (pWpt->id, pVor->id);
	strcpy (pWpt->name, pVor->name);
	pWpt->freq = pVor->freq;
	pWpt->pos = pVor->pos;

	// City and region unknown
	memset (pWpt->city, 0, 40);

	// Search for a waypoint comment for this VOR
	int i = searchWptComment (pData, StoredWptVor, pWpt->id);
	if (i != -1) {
		memcpy (&pWpt->comment, &pData->wptCommentDatabase[i], sizeof(SWptComment));
	} else {
		// No comments found, initialize with defaults
		pWpt->comment.type = StoredWptVor;
		strcpy (pWpt->comment.id, pWpt->id);
		strcpy (pWpt->comment.line1, "____________________");
		strcpy (pWpt->comment.line2, "____________________");
	}
}

void Waypoint::CopyWptFix (SWaypoint  *pFix,
						   SWptFix    *pWpt)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	memset (pWpt->id, 0, 8);
	strncpy (pWpt->id, pFix->name, 5);
	pWpt->pos = pFix->pos;

	memset (pWpt->vorId, 0, 8);
	strncpy (pWpt->vorId, pFix->navaid, 6);
	// Truncate any spaces
	char *p = strchr (pWpt->vorId, ' ');
	if (p) *p = '\0';

	pWpt->vorBearing = pFix->bearingToNavaid;
	pWpt->vorDistance = pFix->distanceToNavaid;

	// Set geographical region
}


void Waypoint::CopyWptUser (SUserWaypoint  *pUser,
							SWptUser       *pWpt)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	memset (pWpt->id, 0, 8);
	strncpy (pWpt->id, pUser->name, 5);
	pWpt->pos = pUser->pos;
}


//
// Search functions
//

void Waypoint::SearchAirportId (char *id, bool force)
{
	SAirport *pAirport = NULL;
	int nIcao = 0;
	int nFaa = 0;

	// The ID passed in may be padded with spaces, truncate the string at the first
	//   space character to ensure a correct match in the search routine below
	char sKey[8];
	strcpy (sKey, id);
	char *p = strchr (sKey, ' ');
	if (p) *p = '\0';

	// For performance reasons, only search when there are at least 3 significant
	//   characters in the key unless the 'force' bool is set
	if ((strlen (sKey) >= 3) || force){

		// Search airport database for ID
		nIcao = APISearchAirportsByICAO (sKey, &pAirport);
		if (nIcao == 0) {
			nFaa = APISearchAirportsByFAA (sKey, &pAirport);
		}

		// Update entry fields if an airport was found
		if ((nIcao != 0) || (nFaa != 0)) {
			CopyWptAirport (pAirport, &wptAirport);
			wptAirport.valid = true;
		} else {
			wptAirport.valid = false;
		}
	} else {
		// No search performed, so airport data is invalid
		wptAirport.valid = false;
	}
}


void Waypoint::SearchNdb (char* id)
{
	SNavaid *pList = NULL;

	int type = NAVAID_TYPE_NDB;
	int rc = APISearchNavaidsByID (id, type, &pList);
	if (rc != 0) {
		CopyWptNdb (pList, &wptNdb);
		wptNdb.valid = true;
	} else {
		wptNdb.valid = false;
	}
	APIFreeNavaid (pList);
}


void Waypoint::SearchVor (char* id)
{
	SNavaid *pList = NULL;

	int type = NAVAID_TYPE_VOR;
	int rc = APISearchNavaidsByID (id, type, &pList);
	if (rc != 0) {
//		char szDebug[80];
//		sprintf (szDebug, "%s VOR found", id);
//		APIDrawNoticeToUser (szDebug, 5);
		CopyWptVor (pList, &wptVor);
		wptVor.valid = true;
	} else {
//		char szDebug[80];
//		sprintf (szDebug, "%s VOR not found", id);
//		APIDrawNoticeToUser (szDebug, 5);
		wptVor.valid = false;
	}
	APIFreeNavaid (pList);
}


void Waypoint::SearchFix (char* id)
{
	SWaypoint *pList = NULL;

	int rc = APISearchWaypointsByName (id, &pList);
	if (rc != 0) {
		CopyWptFix (pList, &wptFix);
		wptFix.valid = true;
	} else {
		wptFix.valid = false;
	}
	APIFreeWaypoint (pList);
}


void Waypoint::SearchUser (char* id)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	wptUser.valid = false;
	for (int i=0; i<pData->nUserWaypoints; i++) {
		if (strcmp (pData->userWpDatabase[i].name, id) == 0) {
			CopyWptUser (&pData->userWpDatabase[i], &wptUser);
			wptUser.valid = true;
		}
	}
}


//
// Search for the specified airport ID
//
void Waypoint::SearchAirportId (char *id)
{
	SGeneric *pGeneric;
	int nIcao = 0;
	int nFaa = 0;

	char sKey[40];

	// Search airport database for ID
	strcpy (sKey, sIdEntry);
	char *p = strchr (sKey, ' ');
	if (p) *p = '\0';
	nIcao = APISearchDatabase ("AIRPORT.DBD",
		"APRTICAO.DBI", sKey, &pGeneric);
	if (nIcao == 0) {
		nFaa = APISearchDatabase ("AIRPORT.DBD",
			"APRTFAA.DBI", sKey, &pGeneric);
	}

	// Update entry fields if an airport was found
	if ((nIcao != 0) || (nFaa != 0)) {
		RefreshAirport (pGeneric);
	}
}


//
// Search for the specified airport name
//
void Waypoint::SearchAirportName (char *sName)
{
	// Search airport database for Name
	char sKey[40];
	strcpy (sKey, sName);
	char *p = strchr (sKey, ' ');
	if (p) *p = '\0';

	SGeneric *pGeneric;
	int nFound = APISearchDatabase ("AIRPORT.DBD", "APRTNAME.DBI", sKey, &pGeneric);

	if (nFound != 0) {
		RefreshAirport (pGeneric);
	}
}

void Waypoint::UpdateWptMenuDisplay (void)
{
	char buffer[25];

	// Line 1, prompt
	strcpy (pShared->line1, "Select waypoint type");

	char sAprt[6];
	if ((crsrMode == CrsrWptMenuAprt) && !crsrFlash) {
		strcpy (sAprt, "    ");
	} else {
		strcpy (sAprt, "apt?");
	}

	char sVor[6];
	if ((crsrMode == CrsrWptMenuVor) && !crsrFlash) {
		strcpy (sVor, "    ");
	} else {
		strcpy (sVor, "vor?");
	}

	char sNdb[6];
	if ((crsrMode == CrsrWptMenuNdb) && !crsrFlash) {
		strcpy (sNdb, "    ");
	} else {
		strcpy (sNdb, "ndb?");
	}

	// Format line 2, apt/vor/ndb menu items
	sprintf (buffer, "%s %s %s", sAprt, sVor, sNdb);
	strcpy (pShared->line2, buffer);


	char sInt[6];
	if ((crsrMode == CrsrWptMenuInt) && !crsrFlash) {
		strcpy (sInt, "    ");
	} else {
		strcpy (sInt, "int?");
	}

	char sUser[6];
	if ((crsrMode == CrsrWptMenuUser) && !crsrFlash) {
		strcpy (sUser, "     ");
	} else {
		strcpy (sUser, "user?");
	}

	// Format line 3, int/user menu items
	sprintf (buffer, "%s %s", sInt, sUser);
	strcpy (pShared->line3, buffer);
}

void Waypoint::UpdateWptProximityDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, page title
	strcpy (pShared->line1, "Proximity waypoints");

	// Display first visible proximity waypoint
	int iWpt = pData->iProxWpt;
	FormatProximityWaypoint (iWpt, pShared->line2);

	// Display second visible proximity waypoint
	iWpt = incProxWaypoint (iWpt);
	FormatProximityWaypoint (iWpt, pShared->line3);
}

void Waypoint::UpdateWptUserCatalogDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	char buffer[25];

	// Line 1, number of user waypoints
	sprintf (buffer, "%3d user waypoints", pData->nUserWaypoints);
	strcpy (pShared->line1, buffer);

	// Init indices of displayed user waypoints
	int i1 = iUserWpt;
	int i2 = iUserWpt + 1;

	// Line 2, first selected user waypoint
	if (crsrFlash && (i1 < pData->nUserWaypoints)) {
		strcpy (pShared->line2, pData->userWpDatabase[i1].name);
		if (pData->userWpDatabase[i1].rte != -1) {
			char sRte[8];
			sprintf (sRte, " in rte %d", pData->userWpDatabase[i1].rte);
			strcat (pShared->line2, sRte);
		}
	} else {
		strcpy (pShared->line2, "");
	}

	// Line 3
	if (i2 < pData->nUserWaypoints) {
		strcpy (pShared->line3, pData->userWpDatabase[i2].name);
		if (pData->userWpDatabase[i2].rte != -1) {
			char sRte[8];
			sprintf (sRte, " in rte %d", pData->userWpDatabase[i2].rte);
			strcat (pShared->line3, sRte);
		}
	} else {
		strcpy (pShared->line3, "");
	}
}


void Waypoint::UpdateWptCommentDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Index into waypoint comment database
	int iWpt = iWptCommentBase;

	// Line 1, page title
	strcpy (pShared->line1, "Wpts with comments");

	// Line 2, Indices i, i+1, i+2
	char s1[8], s2[8], s3[8];

	if ((pData->nWptComments < iWpt) || ((crsrMode == 0) && !crsrFlash)) {
		strcpy (s1, "");
	} else {
		strcpy (s1, pData->wptCommentDatabase[iWpt].id);
	}
	iWpt++;

	if ((pData->nWptComments < iWpt) || ((crsrMode == 1) && !crsrFlash)) {
		strcpy (s2, "");
	} else {
		strcpy (s2, pData->wptCommentDatabase[iWpt].id);
	}
	iWpt++;

	if ((pData->nWptComments < iWpt) || ((crsrMode == 2) && !crsrFlash)) {
		strcpy (s3, "");
	} else {
		strcpy (s3, pData->wptCommentDatabase[iWpt].id);
	}
	iWpt++;

	sprintf (pShared->line2, "%6s %6s %6s", s1, s2, s3);

	// Line 3, Indices i+3, i+4, i+5
	char s4[8], s5[8], s6[8];

	if ((pData->nWptComments < iWpt) || ((crsrMode == 3) && !crsrFlash)) {
		strcpy (s4, "");
	} else {
		strcpy (s4, pData->wptCommentDatabase[iWpt].id);
	}
	iWpt++;

	if ((pData->nWptComments < iWpt) || ((crsrMode == 4) && !crsrFlash)) {
		strcpy (s5, "");
	} else {
		strcpy (s5, pData->wptCommentDatabase[iWpt].id);
	}
	iWpt++;

	if ((pData->nWptComments < iWpt) || ((crsrMode == 5) && !crsrFlash)) {
		strcpy (s6, "");
	} else {
		strcpy (s6, pData->wptCommentDatabase[iWpt].id);
	}
	iWpt++;

	sprintf (pShared->line3, "%6s %6s %6s", s4, s5, s6);
}


void Waypoint::UpdateAptId (char *s)
{
	if (crsr && (crsrMode == CrsrAptIdEdit)) {
		// Init with temporary text entry value
		strcpy (s, textValue);
	} else {
		// Init with actual selected airport value
		if (wptAirport.valid) {
			strcpy (s, wptAirport.id);
		} else {
			strcpy (s, "");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrAptId:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrAptIdEdit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
}


void Waypoint::UpdateAptName (char *s)
{
	WriteLog ("UpdateAptName entry\n");
	if (crsr && (crsrMode == CrsrAptNameEdit)) {
		// Init with temporary text entry value
		strncpy (s, textValue, 20);
	} else {
		// Init with actual selected airport value
		if (wptAirport.valid) {
			strncpy (s, wptAirport.name, 20);
		} else {
			strcpy (s, "");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrAptName:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrAptNameEdit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
	WriteLog ("UpdateAptName exit\n");
}


void Waypoint::UpdateAptCommentLine1 (char *s)
{
	if (crsr && (crsrMode == CrsrAptCommentLine1Edit)) {
		// Init with temporary text entry value
		strcpy (s, textValue);
	} else {
		// Init with actual selected airport value
		if (wptAirport.valid) {
			strcpy (s, wptAirport.comment.line1);
		} else {
			strcpy (s, "");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrAptCommentLine1:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrAptCommentLine1Edit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
}

void Waypoint::UpdateAptCommentLine2 (char *s)
{
	if (crsr && (crsrMode == CrsrAptCommentLine2Edit)) {
		// Init with temporary text entry value
		strcpy (s, textValue);
	} else {
		// Init with actual selected airport value
		if (wptAirport.valid) {
			strcpy (s, wptAirport.comment.line2);
		} else {
			strcpy (s, "");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrAptCommentLine2:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrAptCommentLine2Edit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
}


void Waypoint::UpdateWptAptIdentDisplay (void)
{
	// Line 1, ID and region

	// Initialize the ID to display
	char szId[8];
	UpdateAptId (szId);
	sprintf (pShared->line1, "apt:%-6s", szId);

	// Line 2, city/state
	char szCity[24];
	memset (szCity, 0, 24);
	sprintf (pShared->line2, "%-21s", szCity);

	// Line 3, facility name
	char szName[24];
	memset (szName, 0, 24);
	UpdateAptName (szName);
	sprintf (pShared->line3, "%-21s", szName);
}

void Waypoint::UpdateWptAptPositionDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1: ID, altitude and fuel

	// Initialize the ID to display
	char szId[8];
	UpdateAptId (szId);

	char szAlt[12];
	formatAltitude ((float)wptAirport.pos.alt, pData->altUnits, szAlt);

	char szFuel[12];
	strcpy (szFuel, "fuel");

	sprintf (pShared->line1, "apt:%-4s %6s %-6s", szId, szAlt, szFuel);

	// Line 2, Position
	char szPosition[24];
	if (wptAirport.valid) {
		formatPosition (wptAirport.pos, pData->posUnits, szPosition);
	} else {
		strcpy (szPosition, "");
	}
	sprintf (pShared->line2, " %-22s", szPosition);

	// Line 3: Approach and airspace info
	char szApproach[12];
	char szAirspace[12];

	if (wptAirport.valid) {
		strcpy (szApproach, "vfr");
		strcpy (szAirspace, "class B");
	} else {
		strcpy (szApproach, "");
		strcpy (szAirspace, "");
	}

	sprintf (pShared->line3, " %-3s    %-12s", szApproach, szAirspace);
}

void Waypoint::UpdateWptAptCommDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1: ID and comm index 0

	// Initialize the ID to display
	char szId[8];
	UpdateAptId (szId);

	int i = iAptComm;
	char sFreq[8];

	// Comm 0
	char sComm0[16];
	if (i <= wptAirport.nComms) {
		formatFrequency (wptAirport.comm[i].freq, sFreq);
		sprintf (sComm0, "%4s %s", wptAirport.comm[i].name, sFreq);
	} else {
		strcpy (sComm0, "");
	}
	i++;

	sprintf (pShared->line1, "apt:%-6s %s", szId, sComm0);

	// Line 2, Comm indices 1 and 2
	char sComm1[16];
	if (i <= wptAirport.nComms) {
		formatFrequency (wptAirport.comm[i].freq, sFreq);
		sprintf (sComm1, "%4s %s", wptAirport.comm[i].name, sFreq);
	} else {
		strcpy (sComm1, "");
	}
	i++;

	char sComm2[16];
	if (i <= wptAirport.nComms) {
		formatFrequency (wptAirport.comm[i].freq, sFreq);
		sprintf (sComm2, "%4s %s", wptAirport.comm[i].name, sFreq);
	} else {
		strcpy (sComm2, "");
	}
	i++;

	sprintf (pShared->line2, "%s %s", sComm1, sComm2);

	// Line 3, Comm indices 3 and 4
	char sComm3[16];
	if (i <= wptAirport.nComms) {
		formatFrequency (wptAirport.comm[i].freq, sFreq);
		sprintf (sComm3, "%4s %s", wptAirport.comm[i].name, sFreq);
	} else {
		strcpy (sComm3, "");
	}
	i++;

	char sComm4[16];
	if (i <= wptAirport.nComms) {
		formatFrequency (wptAirport.comm[i].freq, sFreq);
		sprintf (sComm4, "%4s %s", wptAirport.comm[i].name, sFreq);
	} else {
		strcpy (sComm4, "");
	}
	i++;

	sprintf (pShared->line3, "%s %s", sComm3, sComm4);
}



void Waypoint::UpdateWptAptRwyDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	SWptRunway &r = wptAirport.rwy[iAptRwy];

	// Line 1: ID, rwy ID and length

	// Initialize the ID to display
	char szId[8];
	UpdateAptId (szId);

	char sLength[8];
	formatAltitude (r.length, pData->altUnits, sLength);

	sprintf (pShared->line1, "%-6s %s %s", szId, r.id, sLength);

	// Line 2, Surface and lighting
	char sSurf[16];
	formatRwySurface (r.surface, sSurf);

	char sLighting[16];
	strcpy (sLighting, "ft lights");

	sprintf (pShared->line2, "%9s   %9s", sSurf, sLighting);

	// Line 3: ILS/Localizer data
	if (iAptRwyApproach < r.nApproach) {
		char sType[8];
		SWptRwyApproach &a = r.approach[iAptRwyApproach];
		switch (a.type) {
		case WptRwyApprIls:
			strcpy (sType, "ils");
			break;

		case WptRwyApprLoc:
			strcpy (sType, "loc");
			break;

		default:
			strcpy (sType, "???");
		}

		char sFreq[16];
		formatFrequency (a.freq, sFreq);

		sprintf (pShared->line3, "%s      %s  rw%3s",
			sType, sFreq, a.rwyId);
	} else {
		// No approach data for this runway
		strcpy (pShared->line3, "");
	}
}

void Waypoint::UpdateWptAptCommentsDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1: ID

	// Initialize the ID to display
	char szId[8];
	UpdateAptId (szId);
	sprintf (pShared->line1, "apt:%-6s  comments", szId);

	// Line 2: Start of comment
	UpdateAptCommentLine1 (pShared->line2);

	// Line 3 : End of comment
	UpdateAptCommentLine2 (pShared->line3);
}


void Waypoint::UpdateVorId (char *s)
{
	if (crsr && (crsrMode == CrsrVorIdEdit)) {
		// Init with temporary text entry value
		strcpy (s, textValue);
	} else {
		// Init with actual selected 
		if (wptVor.valid) {
			strcpy (s, wptVor.id);
		} else {
			strcpy (s, "");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrVorId:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrVorIdEdit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
}

void Waypoint::UpdateVorCommentLine1 (char *s)
{
	if (crsr && (crsrMode == CrsrVorCommentLine1Edit)) {
		// Init with temporary text entry value
		strcpy (s, textValue);
	} else {
		// Init with actual selected airport value
		if (wptVor.valid) {
			strcpy (s, wptVor.comment.line1);
		} else {
			strcpy (s, "");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrVorCommentLine1:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrVorCommentLine1Edit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
}

void Waypoint::UpdateVorCommentLine2 (char *s)
{
	if (crsr && (crsrMode == CrsrVorCommentLine2Edit)) {
		// Init with temporary text entry value
		strcpy (s, textValue);
	} else {
		// Init with actual selected airport value
		if (wptVor.valid) {
			strcpy (s, wptVor.comment.line2);
		} else {
			strcpy (s, "");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrVorCommentLine2:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrVorCommentLine2Edit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
}


void Waypoint::UpdateWptVorIdentDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1: ID and region

	// Initialize the ID
	char szId[8];
	UpdateVorId (szId);

	char szRegion[8];
	strcpy (szRegion, "REGION");

	sprintf (pShared->line1, "vor:%3s  %-8s", szId, szRegion);

	// Line 2: City and Line 3: Facility name
	char szCity[40];
	char szName[40];
	if (wptVor.valid) {
		strcpy (szCity, "");
		strcpy (szName, wptVor.name);
	} else {
		strcpy (szCity, "");
		strcpy (szName, "");
	}

	strcpy (pShared->line2, szCity);
	strcpy (pShared->line3, szName);
}

void Waypoint::UpdateWptVorPositionDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1: ID and region

	// Initialize the ID
	char szId[8];
	UpdateVorId (szId);

	sprintf (pShared->line1, "vor:%-6s ", szId);

	// Line 2: Position and Line 3: Specific type
	char szPosition[40];
	char szType[20];
	if (wptVor.valid) {
		formatPosition (wptVor.pos, pData->posUnits, szPosition);
		strcpy (szType, "vor");
	} else {
		strcpy (szPosition, "");
		strcpy (szType, "vor");
	}

	sprintf (pShared->line2, " %-20s", szPosition);
	sprintf (pShared->line3, " %-12s", szType);
}

void Waypoint::UpdateWptVorCommentsDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1: ID and region

	// Initialize the ID
	char szId[8];
	UpdateVorId (szId);

	sprintf (pShared->line1, "vor:%-6s  comments", szId);

	// Line 2: Start of comment
	UpdateVorCommentLine1 (pShared->line2);

	// Line 3 : End of comment
	UpdateVorCommentLine2 (pShared->line3);
}

void Waypoint::UpdateNdbId (char *s)
{
	if (crsr && (crsrMode == CrsrNdbIdEdit)) {
		// Init with temporary text entry value
		strcpy (s, textValue);
	} else {
		// Init with actual selected 
		if (wptNdb.valid) {
			strcpy (s, wptNdb.id);
		} else {
			strcpy (s, "");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrNdbId:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrNdbIdEdit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
}

void Waypoint::UpdateNdbCommentLine1 (char *s)
{
	if (crsr && (crsrMode == CrsrNdbCommentLine1Edit)) {
		// Init with temporary text entry value
		strcpy (s, textValue);
	} else {
		// Init with actual selected airport value
		if (wptNdb.valid) {
			strcpy (s, wptNdb.comment.line1);
		} else {
			strcpy (s, "");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrNdbCommentLine1:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrNdbCommentLine1Edit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
}

void Waypoint::UpdateNdbCommentLine2 (char *s)
{
	if (crsr && (crsrMode == CrsrNdbCommentLine2Edit)) {
		// Init with temporary text entry value
		strcpy (s, textValue);
	} else {
		// Init with actual selected airport value
		if (wptNdb.valid) {
			strcpy (s, wptNdb.comment.line2);
		} else {
			strcpy (s, "");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrNdbCommentLine2:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrNdbCommentLine2Edit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
}


void Waypoint::UpdateWptNdbIdentDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1: ID and region

	// Initialize the ID
	char szId[8];
	UpdateNdbId (szId);
	
	char szRegion[8];
	strcpy (szRegion, "REGION");

	sprintf (pShared->line1, "ndb:%-4s  %-6s", szId, szRegion);

	// Line 2: City and state  and Line 3: Facility name
	char szCity[40];
	char szName[40];
	if (wptNdb.valid) {
		strcpy (szCity, "");
		strcpy (szName, wptNdb.name);
	} else {
		strcpy (szCity, "");
		strcpy (szName, "");
	}

	sprintf (pShared->line2, "%-20s", szCity);
	sprintf (pShared->line3, "%-20s", szName);
}

void Waypoint::UpdateWptNdbPositionDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1, ID and frequency

	// Initialize the ID
	char szId[8];
	UpdateNdbId (szId);

	char szFreq[8];
	formatFrequency (wptNdb.freq, szFreq);

	sprintf (pShared->line1, "ndb:%-4s  %-6s", szId, szFreq);

	// Line 2: Position
	char szPosition[40];
	formatPosition (wptNdb.pos, pData->posUnits, szPosition);

	sprintf (pShared->line2, " %-20s", szPosition);

	// Line 3: NDB type
	strcpy (pShared->line3, "");
}

void Waypoint::UpdateWptNdbCommentsDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1: ID and region

	// Initialize the ID
	char szId[8];
	UpdateNdbId (szId);

	sprintf (pShared->line1, "ndb:%-4s  comments", szId);

	// Line 2: Start of comment
	UpdateNdbCommentLine1 (pShared->line2);

	// Line 3 : End of comment
	UpdateNdbCommentLine2 (pShared->line3);
}

void Waypoint::UpdateIntersectionId (char *s)
{
	if (crsr) {
		// Text entry mode is active, ID is temporary text entry value
		strcpy (s, textValue);

		// Replace spaces with underscores, blank cursor if necessary
		for (int i=0; i<=iTextBound; i++) {
			// Replace space with underscore
			if (s[i] == ' ') {
				s[i] = '_';
			}

			// Blank the cursor
			if ((i == iText) && !crsrFlash) {
				s[i] = '_';
			}
		}
	} else {
		// Text entry mode inactive, ID is currently selected VOR ID
		if (wptFix.valid) {
			strcpy (s, wptFix.id);
		} else {
			strcpy (s, "");
		}
	}
}

void Waypoint::UpdateWptIntersectionIdentDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1, ID and region

	// Initialize the ID
	char szId[8];
	UpdateIntersectionId (szId);

	sprintf (pShared->line1, "int:%-4s", szId);

	// Line 2: Position
	char szPosition[40];
	formatPosition (wptFix.pos, pData->posUnits, szPosition);

	sprintf (pShared->line2, " %-20s", szPosition);

	// Line 3: Proximity Data
	strcpy (pShared->line3, "");
}


void Waypoint::UpdateUserWptId (char *s)
{
	if (crsr && (crsrMode == CrsrUserIdEdit)) {
		// Init with temporary text entry value
		strcpy (s, textValue);
	} else {
		// Init with actual selected 
		if (wptUser.valid) {
			strcpy (s, wptUser.id);
		} else {
			strcpy (s, "    ");
		}
	}

	if (crsr) {
		// Implement flashing cursor (either entire field or current char)
		switch (crsrMode) {
		case CrsrUserId:
			// Non-edit mode, blank entire field
			if (!crsrFlash) strcpy (s, "");
			break;

		case CrsrUserIdEdit:
			// Editing ID, blank the current char only and replace space with _
			if (!crsrFlash) {
				if (s[iText] == ' ') {
					s[iText] = '_';
				} else {
					s[iText] = ' ';
				}
			}
			break;

		default:
			// Cursor not on airport ID field, don't blank anything
			break;
		}
	}
}


void Waypoint::UpdateWptUserIdentDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1, ID

	// Initialize the ID
	char szId[8];
	UpdateUserWptId (szId);

	sprintf (pShared->line1, "usr:%-5s", szId);

	// Line 2: Position
	char szPosition[40];
	formatPosition (wptUser.pos, pData->posUnits, szPosition);

	sprintf (pShared->line2, " %-20s", szPosition);

	// Line 3: Bearing/Distance info
	strcpy (pShared->line3, "");
}


void Waypoint::UpdateWptUserNewDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Line 1, new ID
	sprintf (pShared->line1, "usr %-5s is new", sUserKey);

	// Line 2, first menu line
	char sPosn[8];
	if (!crsrFlash && (crsrMode == CrsrUserNewPosn)) {
		strcpy (sPosn, "     ");
	} else {
		strcpy (sPosn, "posn?");
	}

	char sRefWpt[16];
	if (!crsrFlash && (crsrMode == CrsrUserNewRefWpt)) {
		strcpy (sRefWpt, "        ");
	} else {
		strcpy (sRefWpt, "ref wpt?");
	}

	sprintf (pShared->line2, "enter %s %s", sPosn, sRefWpt);

	// Line 3, second menu line
	char sRngBrg[20];
	if (!crsrFlash && (crsrMode == CrsrUserNewRngBrg)) {
		strcpy (sRngBrg, "                  ");
	} else {
		strcpy (sRngBrg, "rng/brg from posn?");
	}

	sprintf (pShared->line3, "%s", sRngBrg);
}


void Waypoint::UpdateWptDisplay (void)
{
	switch (page) {
	case WptMenu:
		UpdateWptMenuDisplay ();
		break;

	case WptProximity:
		UpdateWptProximityDisplay ();
		break;

	case WptUserCatalog:
		UpdateWptUserCatalogDisplay ();
		break;

	case WptComments:
		UpdateWptCommentDisplay ();
		break;

	case WptAptIdent:
		UpdateWptAptIdentDisplay ();
		break;

	case WptAptPosition:
		UpdateWptAptPositionDisplay ();
		break;

	case WptAptComm:
		UpdateWptAptCommDisplay ();
		break;

	case WptAptRwy:
		UpdateWptAptRwyDisplay ();
		break;

	case WptAptComments:
		UpdateWptAptCommentsDisplay ();
		break;

	case WptVorIdent:
		UpdateWptVorIdentDisplay ();
		break;

	case WptVorPosition:
		UpdateWptVorPositionDisplay ();
		break;

	case WptVorComments:
		UpdateWptVorCommentsDisplay ();
		break;

	case WptNdbIdent:
		UpdateWptNdbIdentDisplay ();
		break;

	case WptNdbPosition:
		UpdateWptNdbPositionDisplay ();
		break;

	case WptNdbComments:
		UpdateWptNdbCommentsDisplay ();
		break;

	case WptIntersectionIdent:
		UpdateWptIntersectionIdentDisplay ();
		break;

	case WptUserIdent:
		UpdateWptUserIdentDisplay ();
		break;

	case WptUserNew:
		UpdateWptUserNewDisplay ();
		break;
	}
}


Waypoint::Waypoint (void *p) :
	FsmState (p), page(WptMenu), type(WptAirport),
	crsr(false), crsrElapsed(0), crsrFlash(true), crsrMode(0),
	iUserWpt(0),
	iAptComm(0), iAptRwy(0), iAptRwyApproach (0),
	iWptCommentBase(0),
	wpConfirmPending(false)
{
}


void Waypoint::Activate (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return;

	// Set mode indicator light on
	pShared->liteWpt = true;

	// Reset page to menu
	page = WptMenu;
	type = WptAirport;

	static bool bFirstActivation = true;

	if (bFirstActivation) {
		// This is the first time that WPT mode is activated...initialize various
		//   class attributes that are depending on the active simulator state
		bFirstActivation = false;

		// Get nearest airport
		SAirport ap;
		memset (&ap, 0, sizeof(SAirport));
		float d = NmToFeet (200);
		int nAirport = APIGetNearestAirport (&pData->userPos, &ap, &d);
		if (nAirport != 0) {
			formatAirportId (&ap, sAirportKey);
		} else {
			strcpy (sAirportKey, "KSFO");
		}
		SearchAirportId (sAirportKey, true);

		// Get nearest VOR
		SNavaid vor;
		memset (&vor, 0, sizeof(SNavaid));
		int nVor = APIGetNearestNavaid (&pData->userPos, NAVAID_TYPE_VOR, &vor, &d);
		if (nVor != 0) {
			strcpy (sVorKey, vor.id);
		} else {
			strcpy (sVorKey, "SFO");
		}
		SearchVor (sVorKey);

		// Get nearest NDB
		SNavaid ndb;
		memset (&ndb, 0, sizeof(SNavaid));
		int nNdb = APIGetNearestNavaid (&pData->userPos, NAVAID_TYPE_NDB, &ndb, &d);
		if (nNdb != 0) {
			strcpy (sNdbKey, ndb.id);
		} else {
			strcpy (sNdbKey, "SFO");
		}
		SearchNdb (sNdbKey);

		// Get nearest intersection
		SNavaid fix;
		memset (&fix, 0, sizeof(SNavaid));
		int nFix = APIGetNearestNavaid (&pData->userPos, NAVAID_TYPE_WAYPOINT, &fix, &d);
		if (nFix != 0) {
			strcpy (sFixKey, fix.id);
		} else {
			strcpy (sFixKey, "AAAAA");
		}
		SearchFix (sFixKey);

		// Get first user waypoint
		if (pData->nUserWaypoints > 0) {
			strcpy (sUserKey, pData->userWpDatabase[0].name);
		} else {
			strcpy (sUserKey, "_____");
		}
		SearchUser (sUserKey);
	}

	// Check to see if this activation comes after a D->TO waypoint entry
	if (wpConfirmPending) {
		// There is a pending D->TO waypoint
		wpConfirmPending = false;

		// Verify that the waypoint was actually confirmed (not rejected)
		if (pData->wpActive.confirmed) {

			// Transfer D->TO waypoint to Active Route
			directToRte (pData->userPos, pData->wpActive, &pData->rteActive, pData->autoLegSelect);

			// Active waypoint has been confirmed, automatically switch to NAV mode
			pData->fsm.HandleTrigger (TriggerNAV);
		}
	} else {
		// There is no pending D->TO waypoint to be activated
		UpdateWptDisplay ();
	}

}


void Waypoint::Deactivate (void)
{
	// Set mode indicator light off
	pShared->liteWpt = false;
}


void Waypoint::RescanEntry (void)
{
	// Re-scan for a match of the new key value
  	switch (page) {
	case WptAptIdent:
	case WptAptPosition:
	case WptAptComm:
	case WptAptRwy:
	case WptAptComments:
		switch (crsrMode) {
		case CrsrAptIdEdit:
			SearchAirportId (textValue, false);
			break;

		case CrsrAptNameEdit:
			// SearchAirportName (textValue, false);
			break;
		}
		break;

	case WptIntersectionIdent:
		SearchFix (textValue);
		break;

	case WptVorIdent:
	case WptVorPosition:
	case WptVorComments:
		switch (crsrMode) {
		case CrsrVorIdEdit:
			SearchVor (textValue);
			break;
		}
		break;

	case WptNdbIdent:
	case WptNdbPosition:
	case WptNdbComments:
		switch (crsrMode) {
		case CrsrNdbIdEdit:
			SearchNdb (textValue);
			break;
		}
		break;
	}
}


void Waypoint::HandleKey (char c)
{
	if (crsr) {
		textValue[iText] = c;
		iText++;
		if (iText > iTextBound) iText = iTextBound;
	}
}


bool Waypoint::HandleTrigger (FsmTrigger t)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	bool rc = false;

	switch (t) {
	case TriggerD_TO:
		// DirectTo
		switch (page) {
			case WptAptIdent:
			case WptAptPosition:
			case WptAptComm:
			case WptAptRwy:
			case WptAptComments:
				// Initialize pending airport data
				wpConfirmPending = true;
				pData->replyTrigger = TriggerWPT;

				// Initialize D->TO waypoint with current airport data
				pData->wpActive.type = WptAirport;
				strcpy (pData->wpActive.id, wptAirport.id);
				pData->wpActive.usePos = true;
				pData->wpActive.pos = wptAirport.pos;
				pData->wpActive.confirmed = false;
				pData->wpActive.direct = true;

				// Transition to the Waypoint Confirm state
				pData->fsm.HandleTrigger (TriggerConfirmWaypoint);
				break;

			case WptIntersectionIdent:
				// Initialize pending airport data
				wpConfirmPending = true;
				pData->replyTrigger = TriggerWPT;

				// Initialize D->TO waypoint with current intersection data
				pData->wpActive.type = WptFix;
				strcpy (pData->wpActive.id, wptFix.id);
				pData->wpActive.usePos = true;
				pData->wpActive.pos = wptFix.pos;
				pData->wpActive.confirmed = false;
				pData->wpActive.direct = true;

				// Transition to the Waypoint Confirm state
				pData->fsm.HandleTrigger (TriggerConfirmWaypoint);
				break;

			case WptNdbIdent:
			case WptNdbPosition:
			case WptNdbComments:
				// Initialize pending airport data
				wpConfirmPending = true;
				pData->replyTrigger = TriggerWPT;

				// Initialize D->TO waypoint with current NDB data
				pData->wpActive.type = WptNdb;
				strcpy (pData->wpActive.id, wptNdb.id);
				pData->wpActive.usePos = true;
				pData->wpActive.pos = wptNdb.pos;
				pData->wpActive.confirmed = false;
				pData->wpActive.direct = true;

				// Transition to the Waypoint Confirm state
				pData->fsm.HandleTrigger (TriggerConfirmWaypoint);
				break;

			case WptVorIdent:
			case WptVorPosition:
			case WptVorComments:
				// Initialize pending airport data
				wpConfirmPending = true;
				pData->replyTrigger = TriggerWPT;

				// Initialize D->TO waypoint with current VOR data
				pData->wpActive.type = WptVor;
				strcpy (pData->wpActive.id, wptVor.id);
				pData->wpActive.confirmed = false;
				pData->wpActive.direct = true;

				// Transition to the Waypoint Confirm state
				pData->fsm.HandleTrigger (TriggerConfirmWaypoint);
				break;

			case WptUserIdent:
				// Initialize pending airport data
				wpConfirmPending = true;
				pData->replyTrigger = TriggerWPT;

				// Initialize D->TO waypoint with current User waypoint data
				pData->wpActive.type = WptUser;
				strcpy (pData->wpActive.id, wptUser.id);
				pData->wpActive.usePos = true;
				pData->wpActive.pos = wptUser.pos;
				pData->wpActive.confirmed = false;
				pData->wpActive.direct = true;

				// Transition to the Waypoint Confirm state
				pData->fsm.HandleTrigger (TriggerConfirmWaypoint);

				break;
		}
		break;

	case TriggerCRSR:
		if (crsr) {
			// Cancel editing mode; restore previous value of field
			crsr = false;
			crsrElapsed = 0;
			crsrFlash = true;

			switch (page) {
			case WptAptIdent:
			case WptAptPosition:
			case WptAptComm:
			case WptAptRwy:
			case WptAptComments:
				SearchAirportId (sAirportKey, true);
				break;

			case WptIntersectionIdent:
				SearchFix (sFixKey);
				break;

			case WptNdbIdent:
			case WptNdbPosition:
			case WptNdbComments:
				SearchNdb (sNdbKey);
				break;

			case WptVorIdent:
			case WptVorPosition:
			case WptVorComments:
				SearchVor (sVorKey);
				break;

			case WptUserIdent:
				SearchUser (sUserKey);
				break;
			}
		} else {
			// Enable cursor
			crsr = true;
			crsrElapsed = 0;
			crsrFlash = true;

			switch (page) {
			case WptMenu:
				crsrMode = CrsrWptMenuAprt;
				break;

			case WptAptIdent:
			case WptAptPosition:
			case WptAptComm:
			case WptAptRwy:
			case WptAptComments:
				crsrMode = CrsrAptId;
				break;

			case WptVorIdent:
			case WptVorPosition:
			case WptVorComments:
				crsrMode = CrsrVorId;
				break;

			case WptNdbIdent:
			case WptNdbPosition:
			case WptNdbComments:
				crsrMode = CrsrNdbId;
				break;

			case WptIntersectionIdent:
				crsrMode = CrsrFixId;
				break;

			case WptUserIdent:
				crsrMode = CrsrUserId;
				break;

			case WptUserCatalog:
				strcpy (sUserKey, pData->userWpDatabase[iUserWpt].name);
				break;
			}
		}
		rc = true;
		break;
	
	case TriggerCLR:
		if (crsr) {
			switch (page) {
			case WptUserCatalog:
				// Clear user key
				strcpy (sUserKey, "_____");
				break;

			default:
				// Clear remainder of temporary text entry
				for (int i=iText; i<=iTextBound; i++) {
					textValue[i] = ' ';
				}
			}
		}
		rc = true;
		break;
	
	case TriggerENT:
		if (crsr) {
			crsr = false;

			// Exit crsr mode, copy temporary text entry to the appropriate key field
  			switch (page) {
			case WptMenu:
				switch (crsrMode) {
				case CrsrWptMenuAprt:
					page = WptAptIdent;
					break;

				case CrsrWptMenuVor:
					page = WptVorIdent;
					break;

				case CrsrWptMenuNdb:
					page = WptNdbIdent;
					break;

				case CrsrWptMenuInt:
					page = WptIntersectionIdent;
					break;

				case CrsrWptMenuUser:
					page = WptUserIdent;
					break;
				}
				break;

			case WptAptIdent:
			case WptAptPosition:
			case WptAptComm:
			case WptAptRwy:
			case WptAptComments:
				switch (crsrMode) {
				case CrsrAptIdEdit:
					strcpy (sAirportKey, textValue);
					SearchAirportId (sAirportKey, true);
					break;

				case CrsrAptNameEdit:
					break;

				case CrsrAptCommentLine1Edit:
					// Copy temp text back to comment area
					strcpy (wptAirport.comment.line1, textValue);
					updateWptComment (pData, wptAirport.comment);
					break;

				case CrsrAptCommentLine2Edit:
					// Copy temp text back to comment area
					strcpy (wptAirport.comment.line2, textValue);
					updateWptComment (pData, wptAirport.comment);
					break;
				}
				break;

			case WptIntersectionIdent:
				strcpy (sFixKey, textValue);
				SearchFix (sFixKey);
				break;

			case WptNdbIdent:
			case WptNdbPosition:
			case WptNdbComments:
				switch (crsrMode) {
				case CrsrNdbIdEdit:
					strcpy (sNdbKey, textValue);
					SearchNdb (sNdbKey);
					break;

				case CrsrNdbCommentLine1Edit:
					// Copy temp text back to comment area
					strcpy (wptNdb.comment.line1, textValue);
					updateWptComment (pData, wptNdb.comment);
					break;

				case CrsrNdbCommentLine2Edit:
					// Copy temp text back to comment area
					strcpy (wptNdb.comment.line2, textValue);
					updateWptComment (pData, wptNdb.comment);
					break;
				}
				break;

			case WptVorIdent:
			case WptVorPosition:
			case WptVorComments:
				switch (crsrMode) {
				case CrsrVorIdEdit:
					strcpy (sVorKey, textValue);
					SearchVor (sVorKey);
					break;

				case CrsrVorCommentLine1Edit:
					// Copy temp text back to comment area
					strcpy (wptVor.comment.line1, textValue);
					updateWptComment (pData, wptVor.comment);
					break;

				case CrsrVorCommentLine2Edit:
					// Copy temp text back to comment area
					strcpy (wptVor.comment.line2, textValue);
					updateWptComment (pData, wptVor.comment);
					break;
				}
				break;

			case WptUserCatalog:
				if (strcmp (sUserKey, "_____") == 0) {
					// ID is blank, delete the selected user waypoint
//					userWpDelete (iUserWpt);
				} else {
					// Transition to user wpt details page
//				strcpy (sUserKey, pData->userWpDatabase[iUserWpt].name);
					SearchUser (sUserKey);
					page = WptUserIdent;
				}
				break;

			case WptUserIdent:
				switch (crsrMode) {
				case CrsrUserIdEdit:
					strcpy (sUserKey, textValue);
					SearchUser (sUserKey);

					// If not found, then prompt to add new user waypoint
					if (!wptUser.valid) {
						page = WptUserNew;
						crsr = true;
						crsrMode = CrsrUserNewPosn;
					}
					break;
				}
				break;
			}

			rc = true;
		}
		break;

	case TriggerOuterRight:
		if (crsr) {
			// CRSR active
			switch (page) {
			case WptMenu:
				// Increment WPT menu crsr mode
				incWptMenuCrsr (crsrMode);
				break;

			case WptAptIdent:
				switch (crsrMode) {
				case CrsrAptId:
					crsrMode = CrsrAptName;
					break;

				case CrsrAptName:
					crsrMode = CrsrAptId;
					break;

				case CrsrAptIdEdit:
				case CrsrAptNameEdit:
					// Increment edit index
					iText++;
					if (iText > iTextBound) iText = iTextBound;
					break;
				}
				break;

			case WptAptPosition:
			case WptAptComm:
			case WptAptRwy:
				switch (crsrMode) {
				case CrsrAptIdEdit:
					// Increment edit index
					iText++;
					if (iText > iTextBound) iText = iTextBound;
					break;
				}
				break;

			case WptAptComments:
				// Rotate through fields
				switch (crsrMode) {
				case CrsrAptId:
					crsrMode = CrsrAptCommentLine1;
					break;

				case CrsrAptCommentLine1:
					crsrMode = CrsrAptCommentLine2;
					break;

				case CrsrAptCommentLine2:
					crsrMode = CrsrAptId;
					break;

				case CrsrAptIdEdit:
				case CrsrAptCommentLine1Edit:
				case CrsrAptCommentLine2Edit:
					// Increment crsr index
					iText++;
					if (iText > iTextBound) iText = iTextBound;
					break;
				}
				break;

			case WptVorIdent:
			case WptVorPosition:
				switch (crsrMode) {
				case CrsrVorIdEdit:
					// Increment edit index
					iText++;
					if (iText > iTextBound) iText = iTextBound;
					break;
				}
				break;

			case WptVorComments:
				switch (crsrMode) {
				case CrsrVorId:
					crsrMode = CrsrVorCommentLine1;
					break;

				case CrsrVorCommentLine1:
					crsrMode = CrsrVorCommentLine2;
					break;

				case CrsrVorCommentLine2:
					crsrMode = CrsrVorId;
					break;

				case CrsrVorIdEdit:
				case CrsrVorCommentLine1Edit:
				case CrsrVorCommentLine2Edit:
					// Increment edit index
					iText++;
					if (iText > iTextBound) iText = iTextBound;
					break;
				}
				break;

			case WptNdbIdent:
			case WptNdbPosition:
				switch (crsrMode) {
				case CrsrNdbIdEdit:
					// Increment edit index
					iText++;
					if (iText > iTextBound) iText = iTextBound;
					break;
				}
				break;

			case WptNdbComments:
				switch (crsrMode) {
				case CrsrNdbId:
					crsrMode = CrsrNdbCommentLine1;
					break;

				case CrsrNdbCommentLine1:
					crsrMode = CrsrNdbCommentLine2;
					break;

				case CrsrNdbCommentLine2:
					crsrMode = CrsrNdbId;
					break;

				case CrsrNdbIdEdit:
				case CrsrNdbCommentLine1Edit:
				case CrsrNdbCommentLine2Edit:
					// Increment edit index
					iText++;
					if (iText > iTextBound) iText = iTextBound;
					break;
				}
				break;

			case WptUserIdent:
				// Increment crsr index
				iText++;
				if (iText > iTextBound) iText = iTextBound;
				break;

			case WptUserNew:
				// Cycle through crsr modes
				switch (crsrMode) {
				case CrsrUserNewPosn:
					crsrMode = CrsrUserNewRefWpt;
					break;

				case CrsrUserNewRefWpt:
					crsrMode = CrsrUserNewRngBrg;
					break;

				case CrsrUserNewRngBrg:
					crsrMode = CrsrUserNewPosn;
					break;
				}
				break;

			case WptUserCatalog:
				// Increment user waypoint display index
				if (iUserWpt < (pData->nUserWaypoints-1)) {
					iUserWpt++;
				}
				break;
			}
		} else {
			// CRSR not active, cycle WPT page
			switch (page) {
			case WptMenu:
				page = WptProximity;
				break;

			case WptProximity:
				page = WptUserCatalog;
				break;

			case WptUserCatalog:
				page = WptComments;
				break;

			case WptComments:
				page = WptMenu;
				break;

			case WptAptIdent:
				page = WptAptPosition;
				break;

			case WptAptPosition:
				page = WptAptComm;
				break;

			case WptAptComm:
				page = WptAptRwy;
				break;

			case WptAptRwy:
				page = WptAptComments;
				break;

			case WptAptComments:
				page = WptAptIdent;
				break;

			case WptVorIdent:
				page = WptVorPosition;
				break;

			case WptVorPosition:
				page = WptVorComments;
				break;

			case WptVorComments:
				page = WptVorIdent;
				break;

			case WptNdbIdent:
				page = WptNdbPosition;
				break;

			case WptNdbPosition:
				page = WptNdbComments;
				break;

			case WptNdbComments:
				page = WptNdbIdent;
				break;

			case WptIntersectionIdent:
			case WptUserIdent:
				break;
			}
		}
		rc = true;
		break;

	case TriggerOuterLeft:
		if (crsr) {
			// CRSR active
			switch (page) {
			case WptMenu:
				// Decrement WPT menu crsr mode
				decWptMenuCrsr (crsrMode);
				break;

			case WptAptIdent:
				switch (crsrMode) {
				case CrsrAptId:
					crsrMode = CrsrAptName;
					break;

				case CrsrAptName:
					crsrMode = CrsrAptId;
					break;

				case CrsrAptIdEdit:
				case CrsrAptNameEdit:
					// Increment edit index
					if (iText > 0) iText--;
					break;
				}
				break;

			case WptAptPosition:
			case WptAptComm:
			case WptAptRwy:
				switch (crsrMode) {
				case CrsrAptIdEdit:
					// Increment edit index
					if (iText > 0) iText--;
					break;
				}
				break;

			case WptAptComments:
				// Rotate through fields
				switch (crsrMode) {
				case CrsrAptId:
					crsrMode = CrsrAptCommentLine2;
					break;

				case CrsrAptCommentLine1:
					crsrMode = CrsrAptId;
					break;

				case CrsrAptCommentLine2:
					crsrMode = CrsrAptCommentLine1;
					break;

				case CrsrAptIdEdit:
				case CrsrAptCommentLine1Edit:
				case CrsrAptCommentLine2Edit:
					// Increment crsr index
					if (iText > 0) iText--;
					break;
				}
				break;

			case WptVorIdent:
			case WptVorPosition:
				switch (crsrMode) {
				case CrsrVorIdEdit:
					// Decrement edit index
					if (iText > 0) iText--;
					break;
				}
				break;

			case WptVorComments:
				switch (crsrMode) {
				case CrsrVorId:
					crsrMode = CrsrVorCommentLine2;
					break;

				case CrsrVorCommentLine1:
					crsrMode = CrsrVorId;
					break;

				case CrsrVorCommentLine2:
					crsrMode = CrsrVorCommentLine1;
					break;

				case CrsrVorIdEdit:
				case CrsrVorCommentLine1Edit:
				case CrsrVorCommentLine2Edit:
					// Decrement edit index
					if (iText > 0) iText--;
					break;
				}
				break;

			case WptNdbIdent:
			case WptNdbPosition:
				switch (crsrMode) {
				case CrsrNdbIdEdit:
					// Decrement edit index
					if (iText > 0) iText--;
					break;
				}
				break;

			case WptNdbComments:
				switch (crsrMode) {
				case CrsrNdbId:
					crsrMode = CrsrNdbCommentLine2;
					break;

				case CrsrNdbCommentLine1:
					crsrMode = CrsrNdbId;
					break;

				case CrsrNdbCommentLine2:
					crsrMode = CrsrNdbCommentLine1;
					break;

				case CrsrNdbIdEdit:
				case CrsrNdbCommentLine1Edit:
				case CrsrNdbCommentLine2Edit:
					// Decrement edit index
					if (iText > 0) iText--;
					break;
				}
				break;

			case WptUserIdent:
				// Decrement edit index
				if (iText > 0) iText--;
				break;

			case WptUserNew:
				// Cycle through crsr modes
				switch (crsrMode) {
				case CrsrUserNewPosn:
					crsrMode = CrsrUserNewRngBrg;
					break;

				case CrsrUserNewRefWpt:
					crsrMode = CrsrUserNewPosn;
					break;

				case CrsrUserNewRngBrg:
					crsrMode = CrsrUserNewRefWpt;
					break;
				}
				break;

			case WptUserCatalog:
				// Increment user waypoint display index
				if (iUserWpt > 0) iUserWpt--;
				break;
			}
		} else {
			// CRSR not active, cycle WPT page
			switch (page) {
			case WptMenu:
				page = WptComments;
				break;

			case WptProximity:
				page = WptMenu;
				break;

			case WptUserCatalog:
				page = WptProximity;
				break;

			case WptComments:
				page = WptUserCatalog;
				break;

			case WptAptIdent:
				page = WptAptComments;
				break;

			case WptAptPosition:
				page = WptAptIdent;
				break;

			case WptAptComm:
				page = WptAptPosition;
				break;

			case WptAptRwy:
				page = WptAptComm;
				break;

			case WptAptComments:
				page = WptAptRwy;
				break;

			case WptVorIdent:
				page = WptVorComments;
				break;

			case WptVorPosition:
				page = WptVorIdent;
				break;

			case WptVorComments:
				page = WptVorPosition;
				break;

			case WptNdbIdent:
				page = WptNdbComments;
				break;

			case WptNdbPosition:
				page = WptNdbIdent;
				break;

			case WptNdbComments:
				page = WptNdbPosition;
				break;

			case WptIntersectionIdent:
			case WptUserIdent:
				break;
			}
		}
		rc = true;
		break;

	case TriggerInnerRight:
		if (crsr) {
			// CRSR active
			switch (page) {
			case WptAptIdent:
				switch (crsrMode) {
				case CrsrAptId:
					// Transition to ID edit mode
					crsrMode = CrsrAptIdEdit;
					iText = 0;
					iTextBound = 3;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, sAirportKey);
					break;

				case CrsrAptName:
					// Transition to Name edit mode
					crsrMode = CrsrAptNameEdit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, "                    ");
					break;

				case CrsrAptIdEdit:
				case CrsrAptNameEdit:
					// Increment selected char
					incEntry (textValue[iText]);
					RescanEntry ();
					break;
				}
				break;

			case WptAptPosition:
			case WptAptComm:
			case WptAptRwy:
			case WptAptComments:
				switch (crsrMode) {
				case CrsrAptId:
					// Transition to ID edit mode
					crsrMode = CrsrAptIdEdit;
					iText = 0;
					iTextBound = 3;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, sAirportKey);
					break;

				case CrsrAptCommentLine1:
					// Transition to Comment line 1 edit mode
					crsrMode = CrsrAptCommentLine1Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptAirport.comment.line1);
					break;

				case CrsrAptCommentLine2:
					// Transition to Comment line 2 edit mode
					crsrMode = CrsrAptCommentLine2Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptAirport.comment.line2);
					break;

				case CrsrAptIdEdit:
				case CrsrAptNameEdit:
				case CrsrAptCommentLine1Edit:
				case CrsrAptCommentLine2Edit:
					// Increment selected char
					incEntry (textValue[iText]);
					RescanEntry ();
					break;
				}
				break;

			case WptVorIdent:
			case WptVorPosition:
			case WptVorComments:
				switch (crsrMode) {
				case CrsrVorId:
					// Transition to ID edit mode
					crsrMode = CrsrNdbIdEdit;
					iText = 0;
					iTextBound = 3;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, sVorKey);
					break;

				case CrsrVorCommentLine1:
					// Transition to Comment line 1 edit mode
					crsrMode = CrsrVorCommentLine1Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptVor.comment.line1);
					break;

				case CrsrVorCommentLine2:
					// Transition to Comment line 2 edit mode
					crsrMode = CrsrVorCommentLine2Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptVor.comment.line2);
					break;

				case CrsrVorIdEdit:
				case CrsrVorCommentLine1Edit:
				case CrsrVorCommentLine2Edit:
					// Increment selected char
					incEntry (textValue[iText]);
					RescanEntry ();
					break;
				}
				break;

			case WptNdbIdent:
			case WptNdbPosition:
			case WptNdbComments:
				switch (crsrMode) {
				case CrsrNdbId:
					// Transition to ID edit mode
					crsrMode = CrsrNdbIdEdit;
					iText = 0;
					iTextBound = 3;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, sNdbKey);
					break;

				case CrsrNdbCommentLine1:
					// Transition to Comment line 1 edit mode
					crsrMode = CrsrNdbCommentLine1Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptNdb.comment.line1);
					break;

				case CrsrNdbCommentLine2:
					// Transition to Comment line 2 edit mode
					crsrMode = CrsrNdbCommentLine2Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptNdb.comment.line2);
					break;

				case CrsrNdbIdEdit:
				case CrsrNdbCommentLine1Edit:
				case CrsrNdbCommentLine2Edit:
					// Increment selected char
					incEntry (textValue[iText]);
					RescanEntry ();
					break;
				}
				break;

			case WptUserIdent:
				switch (crsrMode) {
				case CrsrUserId:
					// Transition to ID edit mode
					crsrMode = CrsrUserIdEdit;
					iText = 0;
					iTextBound = 4;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, sUserKey);
					break;

				case CrsrUserIdEdit:
					// Increment selected char
					incEntry (textValue[iText]);
					RescanEntry ();
					break;
				}
				break;
			}
		} else {
			// CRSR not active
			switch (page) {
			case WptProximity:
				// Inner knob increments current prox waypoint
				pData->iProxWpt = incProxWaypoint (pData->iProxWpt);
				break;

			case WptUserCatalog:
				// Increment user waypoint display index
				if (iUserWpt < (pData->nUserWaypoints-1)) {
					iUserWpt++;
				}
				break;

			case WptAptComm:
				// Increment airport comms base index
				if (wptAirport.valid) {
					if (iAptComm < (wptAirport.nComms-5)) {
						iAptComm++;
					}
				}
				break;

			case WptAptRwy:
				// Increment airport runway base index
				IncAptRwy ();
				break;
			}
		}
		rc = true;
		break;

	case TriggerInnerLeft:
		if (crsr) {
			// CRSR active
			switch (page) {
			case WptAptIdent:
				switch (crsrMode) {
				case CrsrAptId:
					// Transition to ID edit mode
					crsrMode = CrsrAptIdEdit;
					iText = 0;
					iTextBound = 3;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, sAirportKey);
					break;

				case CrsrAptName:
					// Transition to Name edit mode
					crsrMode = CrsrAptNameEdit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptAirport.name);
					break;

				case CrsrAptCommentLine1:
					// Transition to Comment line 1 edit mode
					crsrMode = CrsrAptCommentLine1Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptAirport.comment.line1);
					break;

				case CrsrAptCommentLine2:
					// Transition to Comment line 2 edit mode
					crsrMode = CrsrAptCommentLine2Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptAirport.comment.line2);
					break;

				case CrsrAptIdEdit:
				case CrsrAptNameEdit:
				case CrsrAptCommentLine1Edit:
				case CrsrAptCommentLine2Edit:
					// Increment selected char
					decEntry (textValue[iText]);
					RescanEntry ();
					break;
				}
				break;

			case WptVorIdent:
			case WptVorPosition:
			case WptVorComments:
				switch (crsrMode) {
				case CrsrVorId:
					// Transition to ID edit mode
					crsrMode = CrsrNdbIdEdit;
					iText = 0;
					iTextBound = 3;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, sVorKey);
					break;

				case CrsrVorCommentLine1:
					// Transition to Comment line 1 edit mode
					crsrMode = CrsrVorCommentLine1Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptVor.comment.line1);
					break;

				case CrsrVorCommentLine2:
					// Transition to Comment line 2 edit mode
					crsrMode = CrsrVorCommentLine2Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptVor.comment.line2);
					break;

				case CrsrVorIdEdit:
				case CrsrVorCommentLine1Edit:
				case CrsrVorCommentLine2Edit:
					// Increment selected char
					decEntry (textValue[iText]);
					RescanEntry ();
					break;
				}
				break;

			case WptNdbIdent:
			case WptNdbPosition:
			case WptNdbComments:
				switch (crsrMode) {
				case CrsrNdbId:
					// Transition to ID edit mode
					crsrMode = CrsrNdbIdEdit;
					iText = 0;
					iTextBound = 3;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, sNdbKey);
					break;

				case CrsrNdbCommentLine1:
					// Transition to Comment line 1 edit mode
					crsrMode = CrsrNdbCommentLine1Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptNdb.comment.line1);
					break;

				case CrsrNdbCommentLine2:
					// Transition to Comment line 2 edit mode
					crsrMode = CrsrNdbCommentLine2Edit;
					iText = 0;
					iTextBound = 19;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, wptNdb.comment.line2);
					break;

				case CrsrNdbIdEdit:
				case CrsrNdbCommentLine1Edit:
				case CrsrNdbCommentLine2Edit:
					// Increment selected char
					decEntry (textValue[iText]);
					RescanEntry ();
					break;
				}
				break;

			case WptUserIdent:
				switch (crsrMode) {
				case CrsrUserId:
					// Transition to ID edit mode
					crsrMode = CrsrUserIdEdit;
					iText = 0;
					iTextBound = 4;

					// Initialize text entry field to current key, padded with spaces
					memset (textValue, ' ', iTextBound);
					textValue[iTextBound+1] = '\0';
					strcpy (textValue, sUserKey);
					break;

				case CrsrUserIdEdit:
					// Increment selected char
					decEntry (textValue[iText]);
					RescanEntry ();
					break;
				}
				break;
			}
		} else {
			// CRSR not active
			switch (page) {
			case WptProximity:
				// Inner knob decrements current prox waypoint
				pData->iProxWpt = decProxWaypoint (pData->iProxWpt);
				break;

			case WptUserCatalog:
				// Increment user waypoint display index
				if (iUserWpt > 0) {
					iUserWpt--;
				}
				break;

			case WptAptComm:
				// Decrement airport comms base index
				if (wptAirport.valid) {
					if (iAptComm > 0) {
						iAptComm--;
					}
				}
				break;

			case WptAptRwy:
				// Decrement airport runway base index
				DecAptRwy ();
				break;
			}
		}
		rc = true;
		break;

	case TriggerKeyA:
		HandleKey ('A');
		RescanEntry ();
		break;

	case TriggerKeyB:
		HandleKey ('B');
		RescanEntry ();
		break;

	case TriggerKeyC:
		HandleKey ('C');
		RescanEntry ();
		break;

	case TriggerKeyD:
		HandleKey ('D');
		RescanEntry ();
		break;

	case TriggerKeyE:
		HandleKey ('E');
		RescanEntry ();
		break;

	case TriggerKeyF:
		HandleKey ('F');
		RescanEntry ();
		break;

	case TriggerKeyG:
		HandleKey ('G');
		RescanEntry ();
		break;

	case TriggerKeyH:
		HandleKey ('H');
		RescanEntry ();
		break;

	case TriggerKeyI:
		HandleKey ('I');
		RescanEntry ();
		break;

	case TriggerKeyJ:
		HandleKey ('J');
		RescanEntry ();
		break;

	case TriggerKeyK:
		HandleKey ('K');
		RescanEntry ();
		break;

	case TriggerKeyL:
		HandleKey ('L');
		RescanEntry ();
		break;

	case TriggerKeyM:
		HandleKey ('M');
		RescanEntry ();
		break;

	case TriggerKeyN:
		HandleKey ('N');
		RescanEntry ();
		break;

	case TriggerKeyO:
		HandleKey ('O');
		RescanEntry ();
		break;

	case TriggerKeyP:
		HandleKey ('P');
		RescanEntry ();
		break;

	case TriggerKeyQ:
		HandleKey ('Q');
		RescanEntry ();
		break;

	case TriggerKeyR:
		HandleKey ('R');
		RescanEntry ();
		break;

	case TriggerKeyS:
		HandleKey ('S');
		RescanEntry ();
		break;

	case TriggerKeyT:
		HandleKey ('T');
		RescanEntry ();
		break;

	case TriggerKeyU:
		HandleKey ('U');
		RescanEntry ();
		break;

	case TriggerKeyV:
		HandleKey ('V');
		RescanEntry ();
		break;

	case TriggerKeyW:
		HandleKey ('W');
		RescanEntry ();
		break;

	case TriggerKeyX:
		HandleKey ('X');
		RescanEntry ();
		break;

	case TriggerKeyY:
		HandleKey ('Y');
		RescanEntry ();
		break;

	case TriggerKeyZ:
		HandleKey ('Z');
		RescanEntry ();
		break;

	case TriggerKey1:
		HandleKey ('1');
		RescanEntry ();
		break;

	case TriggerKey2:
		HandleKey ('2');
		RescanEntry ();
		break;

	case TriggerKey3:
		HandleKey ('3');
		RescanEntry ();
		break;

	case TriggerKey4:
		HandleKey ('4');
		RescanEntry ();
		break;

	case TriggerKey5:
		HandleKey ('5');
		RescanEntry ();
		break;

	case TriggerKey6:
		HandleKey ('6');
		RescanEntry ();
		break;

	case TriggerKey7:
		HandleKey ('7');
		RescanEntry ();
		break;

	case TriggerKey8:
		HandleKey ('8');
		RescanEntry ();
		break;

	case TriggerKey9:
		HandleKey ('9');
		RescanEntry ();
		break;

	case TriggerKey0:
		HandleKey ('0');
		RescanEntry ();
		break;

	case TriggerKeySpace:
		HandleKey (' ');
		RescanEntry ();
		break;

	case TriggerWPT:
		if (crsr) {
		} else {
			// Re-activate WPT mode if button pressed again
			Activate();
		}
		rc = true;
		break;
	
	}

	// Reset elapsed time since last trigger
	triggerElapsed = 0;

	UpdateWptDisplay ();

	return rc;
}

void Waypoint::TimeSlice (float dT)
{
	static const float CRSR_ON_CYCLE  = (float)0.8;
	static const float CRSR_OFF_CYCLE = (float)0.2;

	// Increment cursor flash elapsed time
	if (crsr) {
		crsrElapsed += dT;
		if (crsrFlash) {
			// CRSR flash is on, delay for CRSR_ON_CYCLE time
			if (crsrElapsed > CRSR_ON_CYCLE) {
				crsrElapsed = 0;
				crsrFlash = false;
				UpdateWptDisplay ();
			}
		} else {
			// CRSR flash is off, delay for CRSR_OFF_CYCLE time
			if (crsrElapsed > CRSR_OFF_CYCLE) {
				crsrElapsed = 0;
				crsrFlash = true;
				UpdateWptDisplay ();
			}
		}
	}

	// Increment elapsed time since last trigger
	triggerElapsed += dT;
}

//
// Waypoint confirm state
//
void WptConfirm::Activate (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	crsrFlash = true;
	crsrElapsed = 0;

	// Initialize database storage lookups
	nOptions = 0;
	pApList = NULL;
	pNavaidList = NULL;

	// Initialize specific data
	SRouteWaypoint &wp = pData->wpActive;
	switch (pData->wpActive.type) {
	case WptAirport:
		{
			// Search for the specified airport
			nOptions = APISearchAirportsByICAO (wp.id, &pApList);
			if (nOptions == 0) {
				// No ICAO airport found, search for FAA
				nOptions = APISearchAirportsByFAA (wp.id, &pApList);
				if (nOptions == 0) {
					// No FAA airport found either
					// The ID may have had a leading 'K' prepended
					nOptions = APISearchAirportsByFAA (wp.id+1, &pApList);
					if (nOptions == 0) {
						wp.confirmed = false;
						pData->fsm.HandleTrigger (pData->replyTrigger);
					}
				}
			}

			// Initialize linked list index to the desired record
			pAp = pApList;
			if (wp.usePos) {
				// A specific waypoint was requested
				while (pAp) {
					if ((pAp->pos.lat == wp.pos.lat) &&
						(pAp->pos.lon == wp.pos.lon) &&
						(pAp->pos.alt == wp.pos.alt))
					{
						break;
					}
					pAp = pAp->next;
				}
				if (!pAp) {
					pAp = pApList;
				}
			}

			// Transfer data from airport record
			wp.pos = pAp->pos;
			strcpy (sId, wp.id);
			formatPosition (pAp->pos, pData->posUnits, sPosition);
			formatAltitude ((float)pAp->pos.alt, pData->altUnits, sAlt);

			int avgasFlags =
				FSIM_AIRPORT_FUEL_80    | FSIM_AIRPORT_FUEL_100 |
				FSIM_AIRPORT_FUEL_100LL | FSIM_AIRPORT_FUEL_115;

			int jetFlags =
				FSIM_AIRPORT_FUEL_JETA      | FSIM_AIRPORT_FUEL_JETA1 |
				FSIM_AIRPORT_FUEL_JETA1PLUS | FSIM_AIRPORT_FUEL_JETB |
				FSIM_AIRPORT_FUEL_JETBPLUS;

			bool avgas = (pAp->fuelTypes & avgasFlags) != 0;
			bool jet = (pAp->fuelTypes & jetFlags) != 0;

			strcpy (sFuel, "");
			if (avgas) {
				if (!jet) {
					strcpy (sFuel, "av gas");
				} else {
					// Both AVGAS and JET available
					strcpy (sFuel, "avgs jet");
				}
			} else {
				if (jet) {
					strcpy (sFuel, "jet");
				}
			}
		}
		break;

	case WptVor:
		{
			nOptions = APISearchNavaidsByID (wp.id, NAVAID_TYPE_VOR, &pNavaidList);

			if (nOptions == 0) {
				wp.confirmed = false;
				pData->fsm.HandleTrigger (pData->replyTrigger);
			}

			// Initialize linked list index to the desired record
			pNavaid = pNavaidList;
			if (wp.usePos) {
				// A specific waypoint was requested, scan through list
				while (pNavaid) {
					if ((pNavaid->pos.lat == wp.pos.lat) &&
						(pNavaid->pos.lon == wp.pos.lon) &&
						(pNavaid->pos.alt == wp.pos.alt))
					{
						break;
					}
					pNavaid = pNavaid->next;
				}
				if (!pNavaid) {
					pNavaid = pNavaidList;
				}
			}

			wp.pos = pNavaid->pos;
			strcpy (sId, wp.id);
			formatAltitude ((float)pNavaid->pos.alt, pData->altUnits, sAlt);
			formatPosition (pNavaid->pos, pData->posUnits, sPosition);
		}
		break;

	case WptNdb:
		{
			nOptions = APISearchNavaidsByID (wp.id, NAVAID_TYPE_NDB, &pNavaidList);
			
			if (nOptions == 0) {
				wp.confirmed = false;
				pData->fsm.HandleTrigger (pData->replyTrigger);
			}

			// Initialize linked list index to the desired record
			pNavaid = pNavaidList;
			if (wp.usePos) {
				// A specific waypoint was requested
				while (pNavaid) {
					if ((pNavaid->pos.lat == wp.pos.lat) &&
						(pNavaid->pos.lon == wp.pos.lon) &&
						(pNavaid->pos.alt == wp.pos.alt))
					{
						break;
					}
					pNavaid = pNavaid->next;
				}

				if (!pNavaid) {
					pNavaid = pNavaidList;
				}
			}

			wp.pos = pNavaid->pos;
			strcpy (sId, wp.id);
			formatAltitude ((float)pNavaid->pos.alt, pData->altUnits, sAlt);
			formatPosition (pNavaid->pos, pData->posUnits, sPosition);
		}
		break;
	}

	// Debug, display how many options would be available
//	char debug[80];
//	sprintf (debug, "nOptions=%d", nOptions);
//	APIDrawNoticeToUser (debug, 5);

	// Initialize display
	UpdateWptConfirmDisplay ();
}


void WptConfirm::Deactivate (void)
{
	// Free any allocated lists
	if (pApList) {
		APIFreeAirport (pApList);
	}

	if (pNavaidList) {
		APIFreeNavaid (pNavaidList);
	}
}


void WptConfirm::UpdateWptConfirmAirportDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, airport name, elevation and available fuel
	sprintf (pShared->line1, "%-4s %7s% -6s", sId, sAlt, sFuel);

	// Line 2, position
	strcpy (pShared->line2, sPosition);

	// Line 3, approach/radar information, and prompt
	char sOk[5];
	if (crsrFlash) {
		strcpy (sOk, "ok?");
	} else {
		strcpy (sOk, "");
	}

	// NOTE: Approach and radar information not available from Fly!
	sprintf (pShared->line3, "                  %-3s", sOk);
}


void WptConfirm::UpdateWptConfirmVorDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, VOR ID and elevation
	sprintf (pShared->line1, "%-4s %8s",
		sId, sAlt);

	// Line 2, position
	strcpy (pShared->line2, sPosition);

	// Line 3, prompt
	char sOk[5];
	if (crsrFlash) {
		strcpy (sOk, "ok?");
	} else {
		strcpy (sOk, "");
	}

	sprintf (pShared->line3, "                %-3s", sOk);
}


void WptConfirm::UpdateWptConfirmNdbDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	// Line 1, NDB ID and elevation
	sprintf (pShared->line1, "%-4s %8s",
		sId, sAlt);

	// Line 2, position
	strcpy (pShared->line2, sPosition);

	// Line 3, prompt
	char sOk[5];
	if (crsrFlash) {
		strcpy (sOk, "ok?");
	} else {
		strcpy (sOk, "");
	}

	sprintf (pShared->line3, "                %-3s", sOk);
}


void WptConfirm::UpdateWptConfirmDisplay (void)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;

	switch (pData->wpActive.type) {
		case WptAirport:
			UpdateWptConfirmAirportDisplay ();
			break;

		case WptVor:
			UpdateWptConfirmVorDisplay ();
			break;

		case WptNdb:
			UpdateWptConfirmNdbDisplay ();
			break;
	}
}


bool WptConfirm::HandleTrigger (FsmTrigger t)
{
	SGps150SystemData *pData = (SGps150SystemData *)pData_m;
	if (!pData) return false;

	bool rc = false;

	switch (t) {
	case TriggerENT:
		pData->wpActive.confirmed = true;
		pData->fsm.HandleTrigger (pData->replyTrigger);
		rc = true;
		break;

	case TriggerCLR:
		pData->wpActive.confirmed = false;
		pData->fsm.HandleTrigger (pData->replyTrigger);
		rc = true;
		break;
	}

	return rc;
}


void WptConfirm::TimeSlice (float dT)
{
	static const float CRSR_ON_CYCLE  = (float)0.8;
	static const float CRSR_OFF_CYCLE = (float)0.2;

	// Increment cursor flash elapsed time
	crsrElapsed += dT;
	if (crsrFlash) {
		// CRSR flash is on, delay for CRSR_ON_CYCLE time
		if (crsrElapsed > CRSR_ON_CYCLE) {
			crsrElapsed = 0;
			crsrFlash = false;
			UpdateWptConfirmDisplay ();
		}
	} else {
		// CRSR flash is off, delay for CRSR_OFF_CYCLE time
		if (crsrElapsed > CRSR_OFF_CYCLE) {
			crsrElapsed = 0;
			crsrFlash = true;
			UpdateWptConfirmDisplay ();
		}
	}
}

//
// Gps150 Subsystem
//
void DLLInstantiate(const long type, const long id, SDLLObject **object)
{
	if ((type == TYPE_DLL_SYSTEM) && (id == 'g150')) {

		// Create DLL object
		*object = APICreateDLLObject();

		WriteLog ("DLLObject: 0x%08X\n", *object);

		// Allocate and initialize object-specific data
		Gps150SystemData *pData = (Gps150SystemData *)APIAllocMem (sizeof(Gps150SystemData));
		(**object).dllObject = pData;
		pSystemData = pData;
		memset (pData, 0, sizeof(Gps150SystemData));

		// Instantiate RealTime data
		pData->pRealTime = (Gps150RealTimeData *)APIAllocMem (sizeof(Gps150RealTimeData));
		memset (pData->pRealTime, 0, sizeof(Gps150RealTimeData));
		pData->realTimeTimer = 0;

		// Initialize power switch to 'on'
		pData->pwrSwitch = true;
		pData->pwrOnTime = 0;

		// Initialize Finite State Machine
		pData->fsm.Init ();
		pData->keyOff       = pData->fsm.AddState (new PowerOff(pData));
		pData->keyPowerUp   = pData->fsm.AddState (new PowerUp(pData));
		pData->keyConfirm   = pData->fsm.AddState (new Confirm(pData));
		pData->keyWptConfirm= pData->fsm.AddState (new WptConfirm(pData));
		pData->keyNrst      = pData->fsm.AddState (new Nearest(pData));
		pData->keySet       = pData->fsm.AddState (new Set(pData));
		pData->keyRte       = pData->fsm.AddState (new Route(pData));
		pData->keyWpt       = pData->fsm.AddState (new Waypoint(pData));
		pData->keyNav       = pData->fsm.AddState (new Nav(pData));
		pData->keyStat      = pData->fsm.AddState (new Status(pData));

		// Transition from PowerOff to PowerUp
		pData->fsm.AddTransition (pData->keyOff, pData->keyPowerUp, TriggerPWR);

		// Transition from PowerUp to Confirm
		pData->fsm.AddTransition (pData->keyPowerUp, pData->keyConfirm, TriggerSelfTestExpiry);

		// Transition from Confirm Databases to StatSatellite
		pData->fsm.AddTransition (pData->keyConfirm, pData->keyStat, TriggerENT);

		// Transition from Status to NAV
		pData->fsm.AddTransition (pData->keyStat, pData->keyNav, TriggerSatAcquisition);

		// Transition to WptConfirm mode 
		pData->fsm.AddTransition (pData->keyWpt, pData->keyWptConfirm, TriggerConfirmWaypoint);
		pData->fsm.AddTransition (pData->keyNrst, pData->keyWptConfirm, TriggerConfirmWaypoint);
		pData->fsm.AddTransition (pData->keyNav, pData->keyWptConfirm, TriggerConfirmWaypoint);

		// Transition to NRST mode 
		pData->fsm.AddTransition (pData->keyWptConfirm, pData->keyNrst, TriggerNRST);
		pData->fsm.AddTransition (pData->keySet, pData->keyNrst, TriggerNRST);
		pData->fsm.AddTransition (pData->keyRte, pData->keyNrst, TriggerNRST);
		pData->fsm.AddTransition (pData->keyWpt, pData->keyNrst, TriggerNRST);
		pData->fsm.AddTransition (pData->keyNav, pData->keyNrst, TriggerNRST);
		pData->fsm.AddTransition (pData->keyStat, pData->keyNrst, TriggerNRST);

		// Transition to SET mode 
		pData->fsm.AddTransition (pData->keyNrst, pData->keySet, TriggerSET);
		pData->fsm.AddTransition (pData->keyRte, pData->keySet, TriggerSET);
		pData->fsm.AddTransition (pData->keyWpt, pData->keySet, TriggerSET);
		pData->fsm.AddTransition (pData->keyNav, pData->keySet, TriggerSET);
		pData->fsm.AddTransition (pData->keyStat, pData->keySet, TriggerSET);

		// Transition to RTE mode 
		pData->fsm.AddTransition (pData->keyNrst, pData->keyRte, TriggerRTE);
		pData->fsm.AddTransition (pData->keySet, pData->keyRte, TriggerRTE);
		pData->fsm.AddTransition (pData->keyWpt, pData->keyRte, TriggerRTE);
		pData->fsm.AddTransition (pData->keyNav, pData->keyRte, TriggerRTE);
		pData->fsm.AddTransition (pData->keyStat, pData->keyRte, TriggerRTE);

		// Transition to WPT mode 
		pData->fsm.AddTransition (pData->keyNrst, pData->keyWpt, TriggerWPT);
		pData->fsm.AddTransition (pData->keySet, pData->keyWpt, TriggerWPT);
		pData->fsm.AddTransition (pData->keyNav, pData->keyWpt, TriggerWPT);
		pData->fsm.AddTransition (pData->keyRte, pData->keyWpt, TriggerWPT);
		pData->fsm.AddTransition (pData->keyStat, pData->keyWpt, TriggerWPT);
		pData->fsm.AddTransition (pData->keyWptConfirm, pData->keyWpt, TriggerWPT);

		// Transition to NAV mode 
		pData->fsm.AddTransition (pData->keyNrst, pData->keyNav, TriggerNAV);
		pData->fsm.AddTransition (pData->keySet, pData->keyNav, TriggerNAV);
		pData->fsm.AddTransition (pData->keyWpt, pData->keyNav, TriggerNAV);
		pData->fsm.AddTransition (pData->keyRte, pData->keyNav, TriggerNAV);
		pData->fsm.AddTransition (pData->keyStat, pData->keyNav, TriggerNAV);
		pData->fsm.AddTransition (pData->keyWptConfirm, pData->keyNav, TriggerNAV);

		// Transition to STAT mode 
		pData->fsm.AddTransition (pData->keyNrst, pData->keyStat, TriggerSTAT);
		pData->fsm.AddTransition (pData->keySet, pData->keyStat, TriggerSTAT);
		pData->fsm.AddTransition (pData->keyWpt, pData->keyStat, TriggerSTAT);
		pData->fsm.AddTransition (pData->keyNav, pData->keyStat, TriggerSTAT);
		pData->fsm.AddTransition (pData->keyRte, pData->keyStat, TriggerSTAT);

		// Set initial FSM state to PowerOff
		pData->fsm.SetCurrentStateKey (pData->keyOff);
		pData->fsm.GetCurrentState()->Activate ();

		// Initialize settings data
		LoadGPSConfiguration (pData);

		// TEMP : Key handler test
		APIAddKeyHandler2 (G150_ID, *object);

		WriteLog ("Instantiated g150 subs object\n");
	}
}


void DLLDestroyObject(SDLLObject *object)
{
	// Initialize local pointer to CRT data
	Gps150SystemData *pData = (Gps150SystemData *)object->dllObject;

	WriteLog ("Destroying g150 subsystem...\n");

	// Store GPS configuration
	SaveGPSConfiguration (pData);

	if (pData) {
		if (pData->pRealTime) {
			APIFreeMem (pData->pRealTime);
		}
		APIFreeMem (pData);
	}
	pData = NULL;

	WriteLog ("Destroyed g150 subs object\n");
}


int	DLLRead (SDLLObject *object, SStream *stream, unsigned int tag)
{
	int	rc = TAG_IGNORED;

	// Initialize local pointer to system data
	Gps150SystemData *pData = (Gps150SystemData *)object->dllObject;
	if (!pData) return rc;

	switch (tag) {
	case 'popw':
		// Popup window width and height
		APIReadUInt (&(pData->winWidth), stream);
		APIReadUInt (&(pData->winHeight), stream);
		WriteLog ("<popw> tag, w=%d  h=%d\n", pData->winWidth, pData->winHeight);
		rc = TAG_READ;
		break;

	case 'sped':
		// Airspeed subsystem message
		APIReadMessage (&(pData->msgSped), stream);
		WriteLog ("<sped> tag\n");
		rc = TAG_READ;
		break;

	case 'alti':
		// Altimeter subsystem message
		APIReadMessage (&(pData->msgAlti), stream);
		WriteLog ("<alti> tag\n");
		rc = TAG_READ;
		break;

	case 'pwrd':
		// Power dependency message
		APIReadMessage (&(pData->msgPower), stream);
		WriteLog ("<pwrd> tag\n");
		rc = TAG_READ;
		break;
	}

	return rc;
}

void DLLPrepare (SDLLObject *object)
{
	// Initialize pointer to system instance data
	Gps150SystemData *pData = (Gps150SystemData *)object->dllObject;
	if (!pData) return;

	// Define standard PMDG hotkeys
	PMDGDefineKeys ();

	// If the subsystem specification included the <popw> tag then bind in
	//   the hotkey to popup the GPS window.  GNS430 uses PMDG Hotkey #4
	if ((pData->winWidth > 0) && (pData->winHeight > 0)) {
		WriteLog ("Adding key handler\n");
		APIAddKeyHandler2 (PMDG_KEY_4, object);
	}

	// Initialize messaging connections
	SMessage *pMsg;
	WriteLog ("Initializing messaging connections...\n");

	// Altimeter
	pMsg = &pData->msgAlti;
	pMsg->id = MSG_GETDATA;
	pMsg->user.u.datatag = 'ialt';
	APISendMessage (pMsg);

	// Power
	pMsg = &pData->msgPower;
	pMsg->id = MSG_GETDATA;
	APISendMessage (pMsg);

	// Airspeed indicator
	pMsg = &pData->msgSped;
	memset (pMsg, 0, sizeof(SMessage));

//	pMsg->id = MSG_GETDATA;
//	pMsg->user.u.datatag = 'kias';
//	APISendMessage (pMsg);


int DLLKeyIntercept(int keyCode, int modifiers)
{
	int rc = false;

	// Initialize pointer to system instance data
	Gps150SystemData *pData = pSystemData;
	if (!pData) return rc;

	// Check for window popup hot-key.  This should also be able to be done in an
	//   implementation of DLLKeyCallback2 but a bug in the SDK prevents this from
	//   working.
	int windowCode, windowModifier;
	APIGetKeyCode(PMDG_KEY_4, &windowCode, &windowModifier);
	if ((keyCode == windowCode) && (modifiers == windowModifier)) {
		if (pData->pWindow) {
			// Window is already created...destroy it
			APICloseWindow (pData->pWindow);
			pData->pWindow = NULL;
		} else {
			// Create popup window display.
			pData->pWindow = APICreateWindow (G150_ID,
				20, 20,
				pData->winWidth, pData->winHeight,
				WINDOW_IS_MOVEABLE);
		}
	}

	// Hotkey definitions are handled below in DLLKeyCallback2,
	//   so this function should only consume keyboard input
	//   when the gauge is in KeyLock mode
	if (pShared->keyLock) {
		
		// Only accept unmodified keys
		unsigned int trigger = 0;
		if (modifiers == 0) {
			switch (keyCode) {
			case KB_KEY_ESC:
				// Esc disables key lock
				pShared->keyLock = false;
				rc = true;
				break;

			case KB_KEY_1:
				trigger = TriggerKey1;
				rc = true;
				break;

			case KB_KEY_2:
				trigger = TriggerKey2;
				rc = true;
				break;

			case KB_KEY_3:
				trigger = TriggerKey3;
				rc = true;
				break;

			case KB_KEY_4:
				trigger = TriggerKey4;
				rc = true;
				break;

			case KB_KEY_5:
				trigger = TriggerKey5;
				rc = true;
				break;

			case KB_KEY_6:
				trigger = TriggerKey6;
				rc = true;
				break;

			case KB_KEY_7:
				trigger = TriggerKey7;
				rc = true;
				break;

			case KB_KEY_8:
				trigger = TriggerKey8;
				rc = true;
				break;

			case KB_KEY_9:
				trigger = TriggerKey9;
				rc = true;
				break;

			case KB_KEY_0:
				trigger = TriggerKey0;
				rc = true;
				break;

			case KB_KEY_BACK:
				// Trigger inner knob right to move to previous char
				trigger = TriggerInnerLeft;
				rc = true;
				break;

			case KB_KEY_Q:
				trigger = TriggerKeyQ;
				rc = true;
				break;

			case KB_KEY_W:
				trigger = TriggerKeyW;
				rc = true;
				break;

			case KB_KEY_E:
				trigger = TriggerKeyE;
				rc = true;
				break;

			case KB_KEY_R:
				trigger = TriggerKeyR;
				rc = true;
				break;

			case KB_KEY_T:
				trigger = TriggerKeyT;
				rc = true;
				break;

			case KB_KEY_Y:
				trigger = TriggerKeyY;
				rc = true;
				break;

			case KB_KEY_U:
				trigger = TriggerKeyU;
				rc = true;
				break;

			case KB_KEY_I:
				trigger = TriggerKeyI;
				rc = true;
				break;

			case KB_KEY_O:
				trigger = TriggerKeyO;
				rc = true;
				break;

			case KB_KEY_P:
				trigger = TriggerKeyP;
				rc = true;
				break;

			case KB_KEY_ENTER:
				trigger = TriggerENT;
				rc = true;
				break;

			case KB_KEY_A:
				trigger = TriggerKeyA;
				rc = true;
				break;

			case KB_KEY_S:
				trigger = TriggerKeyS;
				rc = true;
				break;

			case KB_KEY_D:
				trigger = TriggerKeyD;
				rc = true;
				break;
				
			case KB_KEY_F:
				trigger = TriggerKeyF;
				rc = true;
				break;

			case KB_KEY_G:
				trigger = TriggerKeyG;
				rc = true;
				break;

			case KB_KEY_H:
				trigger = TriggerKeyH;
				rc = true;
				break;

			case KB_KEY_J:
				trigger = TriggerKeyJ;
				rc = true;
				break;

			case KB_KEY_K:
				trigger = TriggerKeyK;
				rc = true;
				break;

			case KB_KEY_L:
				trigger = TriggerKeyL;
				rc = true;
				break;

			case KB_KEY_Z:
				trigger = TriggerKeyZ;
				rc = true;
				break;

			case KB_KEY_X:
				trigger = TriggerKeyX;
				rc = true;
				break;

			case KB_KEY_C:
				trigger = TriggerKeyC;
				rc = true;
				break;

			case KB_KEY_V:
				trigger = TriggerKeyV;
				rc = true;
				break;

			case KB_KEY_B:
				trigger = TriggerKeyB;
				rc = true;
				break;

			case KB_KEY_N:
				trigger = TriggerKeyN;
				rc = true;
				break;

			case KB_KEY_M:
				trigger = TriggerKeyM;
				rc = true;
				break;

			case KB_KEY_SLASH:
				trigger = TriggerKeySlash;
				rc = true;
				break;

			case KB_KEY_SPACE:
				trigger = TriggerKeySpace;
				rc = true;
				break;

			case KB_KEY_GRAY_INS:
			case KB_KEY_INSERT:
				trigger = TriggerD_TO;
				rc = true;
				break;

			case KB_KEY_GRAY_DEL:
			case KB_KEY_DEL:
				trigger = TriggerCLR;
				rc = true;
				break;

			case KB_KEY_KEYPAD_PLUS:
				trigger = TriggerNAV;
				rc = true;
				break;
			
			case KB_KEY_CENTER:
				trigger = TriggerCRSR;
				rc = true;
				break;

			case KB_KEY_GRAY_LEFT:
			case KB_KEY_LEFT:
				trigger = TriggerInnerLeft;
				rc = true;
				break;

			case KB_KEY_GRAY_RIGHT:
			case KB_KEY_RIGHT:
				trigger = TriggerInnerRight;
				rc = true;
				break;

			case KB_KEY_GRAY_UP:
			case KB_KEY_UP:
				trigger = TriggerOuterLeft;
				rc = true;
				break;

			case KB_KEY_GRAY_DOWN:
			case KB_KEY_DOWN:
				trigger = TriggerOuterRight;
				rc = true;
				break;

			case KB_KEY_KEYPAD_ENTER:
				trigger = TriggerENT;
				rc = true;
				break;

			case KB_KEY_GRAY_HOME:
			case KB_KEY_HOME:
				trigger = TriggerNRST;
				rc = true;
				break;

			case KB_KEY_GRAY_END:
			case KB_KEY_END:
				trigger = TriggerSET;
				rc = true;
				break;

			case KB_KEY_GRAY_PGUP:
			case KB_KEY_PGUP:
				trigger = TriggerRTE;
				rc = true;
				break;

			case KB_KEY_GRAY_PGDN:
			case KB_KEY_PGDN:
				trigger = TriggerWPT;
				rc = true;
				break;
			} // switch (keyCode)
		} // if (modifier == 0)

		// If the key was accepted then process the trigger
		if (rc != 0) {
			pData->fsm.HandleTrigger (trigger);
		}
	} // if (pShared->keyLock)

	return rc;
}


void DLLTimeSlice (SDLLObject *object, const float dT)
{
	// Initialize pointer to system instance data
	Gps150SystemData *pData = (Gps150SystemData *)object->dllObject;
	if (!pData) return;

//	WriteLog ("DLLTimeSlice\n");

	// Update user current position and orientation
	APIGetUserObject (&pData->user);
	APIGetObjectPosition (&pData->user, &pData->userPos);
	APIGetObjectOrientation (&pData->user, &pData->userOrient);

	// Increment real-time update timer and refresh data when needed
	pData->realTimeTimer += dT;
	if (pData->realTimeTimer > 1) {
		pData->realTimeTimer = 0;
		updateRealTimeData (pData);
	}

	// Update mag variation if in auto mode
	if (pData->magVarAuto) {
		float f;
		APIGetObjectMagneticVariation(&pData->user, &f);
		pData->magVar = (int)f;
	}

	// Update dependency state
	SMessage *pMsg = &pData->msgPower;
	APISendMessage (pMsg);
	pData->dpndActive = (pMsg->intData != 0);

	// Update power state based on switch state and dependencies
	bool pwrOn = (pData->pwrSwitch && pData->dpndActive);
	if (pwrOn != pShared->onOff) {
		// Power on/off state has changed
		pShared->onOff = pwrOn;

		if (pwrOn) {
			// Power has transitioned to On, set state to PowerUp
			pData->fsm.GetCurrentState()->Deactivate();
			pData->fsm.SetCurrentStateKey (pData->keyPowerUp);
			pData->fsm.GetCurrentState()->Activate();
		} else {
			// Power has transitioned to Off, set state to PowerOff
			pData->fsm.GetCurrentState()->Deactivate();
			pData->fsm.SetCurrentStateKey (pData->keyOff);
			pData->fsm.GetCurrentState()->Activate();
		}
	}

	//
	// Update common GPS data fields
	//

	// Update UTC and local time of day
	pData->utcDate = APIGetDate ();
	pData->utcTime = APIGetTime ();

	// Update power-on time, this will automatically trigger transition from PowerUp
	if (pShared->onOff) {
		pData->pwrOnTime += dT;

		// Check for departure threshold crossing
		if (!pData->departed) {
			if (pData->tripTimerPwrOn) {
				// Trip timer starts with unit power-on
				pData->departed = true;
				pData->utcDepart = pData->utcTime;
			} else {
				// Check user airspeed
				float ias = getUserTrueAirspeed (pData);
				if (ias > pData->tripTimerSpeed) {
					pData->departed = true;
					pData->utcDepart = pData->utcTime;
				}
			}
		}
	} else {
		pData->pwrOnTime = 0;
		pData->departed = false;
	}

	// Increment trip timer if appropriate
	if (pData->departed) {
		pData->tripTimer += dT;
	}

	// Update approach timer
	if (pData->apprTimerActive) {
		if (pData->apprTimerCountDown) {
			// Approach timer in count-down mode
			pData->apprTimer -= dT;
			if (pData->apprTimer <= 0) {
				// Count-down timer expired, transition to count-up
				pData->apprTimerCountDown = false;
			}
		} else {
			// Approach timer in count-up mode
			pData->apprTimer += dT;
			if (pData->apprTimer >= pData->apprTimerPreset) {
				// Count-up timer expired, disable
				pData->apprTimerActive = false;
			}
		}
	}

	// Update active route leg
	if (pData->autoLegSeq) {
		static float prevDistance = 10;
		static bool armTransition = false;

		// Check distance to 'from' waypoint.  When distance is closer than 5 NM,
		//   'arm' the leg transition flag so that when the distance starts
		//   increasing, the leg is transitioned
		SRteActive *pActive = &pData->rteActive;
		if (pActive->valid) {
			if (pActive->iLeg < pActive->nLegs) {

				float d = APIGreatCircleDistance (
					&pData->userPos,
					&pActive->stored.waypoint[pActive->iLeg].pos);

//				char debug[80];
//				sprintf (debug, "iFrom=%d iTo=%d  d=%f  prev=%f",
//					pActive->iFrom, pActive->iTo, FeetToNm (d), FeetToNm(prevDistance));
//				APIDrawNoticeToUser (debug, 1);

				// Check for transition
				if (d <= prevDistance) {
					// Approaching 'To' waypoint

					if (d < NmToFeet (2)) {
						// Approaching within 2 nm of 'To' waypoint, arm the
						//   leg transition flag so that as soon as we are
						//   departing, the leg will advance
						armTransition = true;
					}
				} else {
					// Departing 'To' waypoint

					if (armTransition) {
						// Transition is armed, so do it

//						char debug[80];
//						sprintf (debug, "transition leg %d-%d to %d-%d",
//							pActive->iFrom, pActive->iTo,
//							pActive->iFrom+1, pActive->iTo+1);
//						APIDrawNoticeToUser (debug, 5);

						pActive->iLeg++;
						armTransition = false;
					} else {
						// Moving farther from 'To' waypoint, but the transition
						//   is not armed, so do nothing
					}
				}
				prevDistance = d;
			}
		}
	}

//	WriteLog ("DLLTimeSlice calling state timeslice\n");

	// Call TimeSlice method of active state
	FsmState *pState = pData->fsm.GetCurrentState();
	if (pState) pState->TimeSlice (dT);

//	WriteLog ("DLLTimeSlice done\n");
}


int DLLReceiveMessage (SDLLObject *object, SMessage *msg)
{
	Gps150SystemData *pData = (Gps150SystemData *)object->dllObject;

	// Initialize return code, assume message is not processed
	int rc = MSG_IGNORED;

	if (msg->group == G150_ID) {

		// Message is addressed to this object
		if (msg->id == MSG_SETDATA) {

			switch (msg->user.u.datatag) {
			case TriggerPWR:
				// Toggle unit power
				pData->pwrSwitch = !pData->pwrSwitch;
				rc = MSG_PROCESSED;
				break;

			case TriggerKeyLock:
				// Toggle key lock
				pShared->keyLock = !pShared->keyLock;
				break;

			default:
				// Send the trigger to the active FSM state
				pData->fsm.HandleTrigger (msg->user.u.datatag);
				rc = MSG_PROCESSED;
			}
		} else if (msg->id == MSG_GETDATA) {
			switch (msg->user.u.datatag) {
			case 'shar':
				msg->voidData = (void *)pShared;
				rc = MSG_PROCESSED;
				break;
			}
		}
	}

	return rc;
}


/////////////////////////////////////////////////////////////
//
//	DLL Setup (Init & Kill)
//
//	These functions MUST be implemented, or the DLL
//	will totally fail!!!!
//
/////////////////////////////////////////////////////////////

int	DLLInit(DLLFunctionRegistry *dll,
			SDLLCopyright *copyright,
			SDLLRegisterTypeList **types)
{
	//
	//	populate copyright info
	//
	strcpy(copyright->product, "Garmin GPS150 System");
	strcpy(copyright->company, "Chris Wallace");
	strcpy(copyright->programmer, "Chris Wallace");
	strcpy(copyright->dateTimeVersion, "1.00");
	strcpy(copyright->email, "cwallace@precisionmanuals.com");
	copyright->internalVersion = 100;

	//
	//	return function pointers of available DLL functions
	//
	dll->DLLInstantiate = DLLInstantiate;
	dll->DLLDestroyObject = DLLDestroyObject;
	dll->DLLRead = DLLRead;
	dll->DLLPrepare = DLLPrepare;
	dll->DLLKeyIntercept = DLLKeyIntercept;
	dll->DLLTimeSlice = DLLTimeSlice;
	dll->DLLReceiveMessage = DLLReceiveMessage;

	//	Register all supported object types
	APIAddRegisteredType (types, TYPE_DLL_SYSTEM, G150_ID);

	// Initialize system data pointer
	pSystemData = NULL;

	// Allocate and initialize shared data pointer
	pShared = (Gps150Data *)APIAllocMem (sizeof(Gps150Data));
	pShared->onOff = 0;
	pShared->liteNrst =0;
	pShared->liteSet = 0;
	pShared->liteRte = 0;
	pShared->liteWpt = 0;
	pShared->liteNav = 0;
	pShared->liteMsg = 0;
	pShared->keyLock = false;

	// Open log file
	OpenLog ("g150subs.log");
	WriteLog ("---- Subsystem log opened\n");
	return(1);
}

void DLLKill(void)
{
	if (pShared) {
		APIFreeMem (pShared);
	}

	WriteLog ("---- Subsystem log closed\n");
	CloseLog ();
}

*/