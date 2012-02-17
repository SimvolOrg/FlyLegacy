//=========================================================================================
// SqlMGR
//
// Part of Fly! Legacy project
//
// Copyright 2003-2005 Chris Wallace
// copyright 2007-2008 Jean Sabatier
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
//=========================================================================================
#ifndef SQLMGR_H
#define SQLMGR_H
//=====================================================================================
#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../sqlite/sqlite3.h"
//=====================================================================================
class CWPoint;
class CAirport;
class C_QGT;
class C_CDT;
class C3DPart;
class C3Dmodel;
class CPaveRWY;
class CBaseLITE;
class CArtParser;
class CBlendTexture;
class TCacheMGR;
class CWobj;
class C3DLight;
class C_STile;
class TRN_HDTL;
struct TRACK_EDIT;
struct ELV_PATCHE;
//=====================================================================================
//  DEFINE COLUMN INDICES FOR AIRPORT
//=====================================================================================
#define CLN_APT_TILE 0				    /* Tile key (X-Z) (int) */
#define CLN_APT_UKEY 1			      /* AIRPORT KEY    (10c) */
#define CLN_APT_AFAA 2            /* FAA ID         ( 4c) */
#define CLN_APT_AICA 3            /* AICA ID        ( 6c) */
#define CLN_APT_ANAM 4            /* ANAM           (40c) */
#define CLN_APT_ACTR 5            /* ACTR           ( 4c) */
#define CLN_APT_ASTA 6            /* ASTA           ( 4c) */
#define CLN_APT_ACTY 7            /* ACTY           (20c) */ /* Not used */
#define CLN_APT_ATYP 8           /* ATYP           (int) */
#define CLN_APT_ALAT 9           /* ALAT           (dbl) */
#define CLN_APT_ALON 10           /* ALON           (dbl) */
#define CLN_APT_AELE 11           /* AELE           (int) */
#define CLN_APT_AOWN 12           /* AOWN           (int) */
#define CLN_APT_AUSE 13           /* AUSE           (int) */
#define CLN_APT_ARGN 14           /* AREG           ( 6c) */  /* Not used*/
#define CLN_APT_ANTM 15           /* ANTM           (20c) */  /* Not used */
#define CLN_APT_AFSS 16           /* AFSS           (20c) */  /* Not used */
#define CLN_APT_AMAG 17           /* AMAG           (dbl) */
#define CLN_APT_AALT 18           /* AALT           (int) */
#define CLN_APT_AFSA 19           /* AFSA           (int) */
#define CLN_APT_ANTD 20           /* ANTD           (int) */
#define CLN_APT_AATA 21           /* AATA           (int) */
#define CLN_APT_ASEG 22           /* ASEG           (int) */
#define CLN_APT_ALND 23           /* ALND           (int) */
#define CLN_APT_AMJC 24           /* AMCJ           (int) */
#define CLN_APT_AMLN 25           /* AMLN           (int) */  /* NOT USED */
#define CLN_APT_ACUS 26           /* ACUS           (int) */  /* NOT USED */
#define CLN_APT_AFUE 27           /* AFUE           (int) */
#define CLN_APT_AFRM 28           /* AFRM           (int) */
#define CLN_APT_AENG 29           /* AENG           (int) */
#define CLN_APT_ABTO 30           /* ABTO           (int) */
#define CLN_APT_ABLO 31           /* ABLO           (int) */
#define CLN_APT_ALEN 32           /* Beacon color		(int)	*/
#define CLN_APT_ACFT 33           /* BASED Aircraft types	              (int)	*/
#define CLN_APT_AACO 34           /* Annual commercial operations	      (int) */ // NOT USED
#define CLN_APT_AAGA 35           /* Annual general aviation operations	(int) */ // NOT USED
#define CLN_APT_AAMO 36           /* Annual military operations	        (int) */ // NOT USED
#define CLN_APT_AATF 37           /* Airport attendance		              (int) */
#define CLN_APT_AATS 38           /* Attendance start time	            (int) */
#define CLN_APT_AATE 39           /* Attendance end time		            (int) */
#define CLN_APT_ALTF 40           /* Airport lighting		                (int) */
#define CLN_APT_ALTS 41           /* Airport lighting start time	      (int) */
#define CLN_APT_ALTE 42           /* Airport lighting end time	        (int) */
#define CLN_APT_ATYC 43           /* Airport type			                  (int) */
#define CLN_APT_AICN 44           /* Icon		                            (int) */
#define CLN_APT_ANRW 45           /* Number of runway	                  (int) */
#define CLN_APT_ALRW 46           /* Longuest runway in feet	          (int) */
#define CLN_APT_AVOR 47           /* VOR on airport		                  (int) */
#define CLN_APT_AATC 48           /* ATC Available		                  (int) */
#define CLN_APT_AILS 49           /* ILS Available		                  (int) */
//====================================================================================
//  DEFINE COLUMN INDICES FOR RUNWAYS
//=====================================================================================
#define CLN_RWY_NSEQ  0           /* Sequence number		                (int)   */
#define CLN_RWY_RAPT  1           /* Airport key			                  ( 10c)  */
#define CLN_RWY_RLEN  2           /* Runway length		                  (int)   */
#define CLN_RWY_RWID  3           /* Runway width                       (int)   */
#define CLN_RWY_RSFC  4           /* Surface type			                  (int)   */
#define CLN_RWY_RCON  5           /* Surface condition		              (int)   */
#define CLN_RWY_RPAV  6           /* Pavement			                      (int)   */
#define CLN_RWY_RPCN  7           /* Pavement classification	          (int)   */  // Ignored
#define CLN_RWY_RSUB  8           /* Pavement subgrade		              (int)   */  // Ignored
#define CLN_RWY_RPSI  9           /* Pavement Tire PSI		              (int)   */
#define CLN_RWY_RPAT  10          /* Traffic Pattern direction	        (int)   */
#define CLN_RWY_RCLS  11          /* Closed UNUSABLE		                (int)   */
#define CLN_RWY_RPCL  12          /* Pilot controlled lighting	        (int)   */
/*--- RUNWAY HI END ------------------------------------------------------------*/
#define CLN_RWY_RHID  13          /* Hi End ID			                    (  4c)  */
#define CLN_RWY_RHLA  14          /* LAtitude			                      (dbl)   */
#define CLN_RWY_RHLO  15          /* Longitude			                    (dbl)   */
#define CLN_RWY_RHHT  16          /* Elevation			                    (int)   */
#define CLN_RWY_RHHD  17          /* True heading			                  (dbl)   */
#define CLN_RWY_RHMH  18          /* Magnetic heading		                (dbl)   */
#define CLN_RWY_RHDT  19          /* Displaced threshold		            (int)   */
#define CLN_RWY_RHTE  20          /* Threshold elevation (feet)	        (int)   */ // Ignored
#define CLN_RWY_RHEL  21          /* End lights			                    (char)  */
#define CLN_RWY_RHAL  22          /* Alignment lights		                (char)  */
#define CLN_RWY_RHCL  23          /* Center lights		                  (char)  */
#define CLN_RWY_RHTL  24          /* Touch down lights		              (char)  */
#define CLN_RWY_RHTH  25          /* Threhold lights		                (char)  */
#define CLN_RWY_RHLI  26          /* Edge lights			                  (char)  */
#define CLN_RWY_RHSL  27          /* Flashing lights		                (char)  */
#define CLN_RWY_RHMK  28          /* Markings			                      (char)  */
#define CLN_RWY_RHVR  29          /* RVR				                        (char)  */  // Not used
#define CLN_RWY_RHVV  30          /* RVV				                        (int)   */  // Not used
#define CLN_RWY_RHGT  31          /* Glide slope type		                (char)  */  // Not Used
#define CLN_RWY_RHGC  32          /* Glide slope config		              (char)  */  // Not Used
#define CLN_RWY_RHGL  33          /* Glide slope location		            (char)  */  // Not used
#define CLN_RWY_RHL1  34          /* Ligth system 1		                  (char)  */
#define CLN_RWY_RHL2  35          /* Light system 2		                  (char)  */
#define CLN_RWY_RHL3  36          /* Light system 3		                  (char)  */
#define CLN_RWY_RHL4  37          /* Light system 4		                  (char)  */
#define CLN_RWY_RHL5  38          /* Light system 5		                  (char)  */
#define CLN_RWY_RHL6  39          /* Light system 6		                  (char)  */
#define CLN_RWY_RHL7  40          /* Light system 7		                  (char)  */
#define CLN_RWY_RHL8  41          /* Light system 8		                  (char)  */		
/*--- RUNWAY LO END ------------------------------------------------------------*/	
#define CLN_RWY_RLID  42          /* Hi End ID			                    ( 4c)   */
#define CLN_RWY_RLLA  43          /* LAtitude			                      (dbl)   */
#define CLN_RWY_RLLO  44          /* Longitude			                    (dbl)   */
#define CLN_RWY_RLHT  45          /* Elevation			                    (int)   */
#define CLN_RWY_RLHD  46          /* True heading			                  (dbl)   */
#define CLN_RWY_RLMH  47          /* Magnetic heading		                (dbl)   */
#define CLN_RWY_RLDT  48          /* Displaced threshold		            (int)   */
#define CLN_RWY_RLTE  49          /* Threshold elevation (feet)	        (int)   */
#define CLN_RWY_RLEL  50          /* End lights			                    (char)  */
#define CLN_RWY_RLAL  51          /* Alignment lights		                (char)  */
#define CLN_RWY_RLCL  52          /* Center lights		                  (char)  */
#define CLN_RWY_RLTL  53          /* Touch down lights		              (char)  */
#define CLN_RWY_RLTH  54          /* Threhold lights		                (char)  */
#define CLN_RWY_RLLI  55          /* Edge lights			                  (char)  */
#define CLN_RWY_RLSL  56          /* Flashing lights		                (char)  */
#define CLN_RWY_RLMK  57          /* Markings			                      (char)  */
#define CLN_RWY_RLVR  58          /* RVR				                                */ // Not used
#define CLN_RWY_RLVV  59          /* RVV				                         (int)  */ // Not used
#define CLN_RWY_RLGT  60          /* Glide slope type		                (char)  */ // Not Used
#define CLN_RWY_RLGC  61          /* Glide slope config		              (char)  */  // Not Used
#define CLN_RWY_RLGL  62          /* Glide slope location		            (char)  */  // Not used
#define CLN_RWY_RLL1  63          /* Ligth system 1		                  (char)  */
#define CLN_RWY_RLL2  64          /* Light system 2		                  (char)  */
#define CLN_RWY_RLL3  65          /* Light system 3		                  (char)  */
#define CLN_RWY_RLL4  66          /* Light system 4		                  (char)  */
#define CLN_RWY_RLL5  67          /* Light system 5		                  (char)  */
#define CLN_RWY_RLL6  68          /* Light system 6		                  (char)  */
#define CLN_RWY_RLL7  69          /* Light system 7		                  (char ) */
#define CLN_RWY_RLL8  70          /* Light system 8		                  (char)  */
/*------------------------------------------------------------------------------*/
//====================================================================================
//  DEFINE COLUMN INDICES FOR NAVAID
//====================================================================================
#define CLN_NAV_TILE  0           /* Tile key (X-Z) 	                  (int)   */
#define CLN_NAV_UKEY  1           /* NAVAID KEY  		                    ( 10c)  */
#define CLN_NAV_NAID  2           /* NAV ID ID      	                  (  5c)  */
#define CLN_NAV_NAME  3           /* NAV name  		                      ( 40c)  */
#define CLN_NAV_NFRQ  4           /* Nav frequency	                    (dble)  */
#define CLN_NAV_NMDV  5           /* mag deviation	                    (dble)  */
#define CLN_NAV_NSVR  6           /* Slaved deviation	                  (dble)  */
#define CLN_NAV_NCTY  7           /* Country		                        (  4c)  */
#define CLN_NAV_NSTA  8           /* State		                          (  4c)  */
#define CLN_NAV_NTYP  9           /* Type			                          ( int)  */
#define CLN_NAV_NCLS  10          /* Classification	                    ( int)  */
#define CLN_NAV_NUSE  11          /* usage		                          ( int)  */
#define CLN_NAV_NLAT  12          /* Latitude		                        (dble)  */
#define CLN_NAV_NLON  13          /* Longitude		                      (dble)  */
#define CLN_NAV_NALT  14          /* Altitude		                        (int)   */
#define CLN_NAV_NPOW  15          /* Power ?		                        (dble)  */
#define CLN_NAV_NRGN  16          /* Range		                          (dble)  */	
//====================================================================================
//  DEFINE COLUMN INDICES FOR ILS
//====================================================================================
#define CLN_ILS_TILE  0           /* Tile key (X-Z) 	                  (int)   */
#define CLN_ILS_IKEY  1           /* ILS key		                        ( 10c)  */
#define CLN_ILS_IAPT  2           /* Airport key		                    ( 10c)  */
#define CLN_ILS_IRWY  3           /* Runway end		                      (  4c)  */
#define CLN_ILS_IILS  4           /* ILS identity		                    (  6c)  */
#define CLN_ILS_NAME  5           /* ILS name		                        ( 40c)  */
#define CLN_ILS_NTYP  6           /* ILS TYPE		                        ( int)  */
#define CLN_ILS_ILAT  7           /* Latitude		                        (dble)  */
#define CLN_ILS_ILON  8           /* Longitude		                      (dble)  */
#define CLN_ILS_IALT  9           /* Altitude		                        ( int)  */
#define CLN_ILS_FREQ  10          /* Frequency		                      (dble)  */
#define CLN_ILS_GSAN  11          /* Glide angle		                    (dble)  */
#define CLN_ILS_MDEV  12          /* Mag deviation	                    (dble)  */
#define CLN_ILS_RANG  13          /* Range		                          (dble)  */
//====================================================================================
//  DEFINE COLUMN INDICES FOR COM
//====================================================================================
#define CLN_COM_TILE  0           /* Tile key (X-Z) 	                  (int )  */
#define CLN_COM_CKEY  1           /* COM key		                        ( 10c)  */
#define CLN_COM_CAPT  2           /* Airport key                        ( 10c)  */
#define CLN_COM_CNAM  3           /* COM name                           ( 20c)  */
#define CLN_COM_CTYP  4           /* COM type                           ( Int)  */
#define CLN_COM_FRQ1  5           /* Freq 1                             (dble)  */
#define CLN_COM_FRQ2  6           /* Freq 2                             (dble)  */
#define CLN_COM_FRQ3  7           /* Freq 3                             (dble)  */
#define CLN_COM_FRQ4  8           /* Freq 4                             (dble)  */
#define CLN_COM_FRQ5  9           /* Freq 5                             (dble)  */
#define CLN_COM_CLAT  10          /* Latitude                           (dble)  */
#define CLN_COM_CLON  11          /* Longitde                           (dble)  */
//====================================================================================
//  DEFINE COLUMN INDICES FOR WPT
//====================================================================================
#define CLN_WPT_TILE  0           /* Tile key (X-Z) 	                (int )  */
#define CLN_WPT_WIDN  1           /* Ident                            ( 10c)  */
#define CLN_WPT_UKEY  2           /* WPT Key                          ( 10c)  */
#define CLN_WPT_WNAM  3           /* Name                             ( 26c)  */
#define CLN_WPT_WCTY  4           /* Country                          (  3c)  */
#define CLN_WPT_WSTA  5           /* State		                        (  3c)  */
#define CLN_WPT_WLOC  6           /* Associated VOR                   (  1c)  */
#define CLN_WPT_WTYP  7           /* Type                             ( int)  */
#define CLN_WPT_WUSE  8           /* Usage                            ( int)  */
#define CLN_WPT_WLAT  9           /* Latitude                         (dble)  */
#define CLN_WPT_WLON  10          /* Longitude                        (dble)  */
#define CLN_WPT_WALT  11          /* Altitude                         ( int)  */
#define CLN_WPT_WMAG  12          /* Mag deviation                    (dble)  */
#define CLN_WPT_WBRG  13          /* Bearing to NAV                   (dble)  */
#define CLN_WPT_WDIS  14          /* Distance to NAV                  (dble)  */
#define CLN_WPT_WNAV  15          /* NAV key                           ( 10c) */
//====================================================================================
//  DEFINE COLUMN INDICES FOR COUNTRY
//====================================================================================
#define CLN_CTY_CUID  0           /* Country ID 		                  (   4c) */
#define CLN_CTY_CNAM  1           /* Country name                     ( 40c)  */
//====================================================================================
//  DEFINE COLUMN INDICES FOR STATE
//====================================================================================
#define CLN_STA_SKEY  0           /*  State key                       (  5c)  */
#define CLN_STA_SCTY  1           /* Country code                     (  3c)  */
#define CLN_STA_NSTA  2           /* Abreviation                      (  4c)  */
#define CLN_STA_NAME  3           /*  State name	                    ( 40c)  */
//=====================================================================================
//  DEFINE COLUMN INDICES FRO REGION
//=====================================================================================
#define CLN_REG_QGT   0           // QGT Key
#define CLN_REG_DET   1           // Detail tile key
#define CLN_REG_LGX   2           // X size
#define CLN_REG_LGZ   3           // Z size
#define CLN_REG_SUB   4           // region dimension
#define CLN_REG_TYP   5           // region type
#define CLN_REG_VAL   6           // Region value
//=====================================================================================
#define CLN_HTR_KEY   0           // Key
#define CLN_HTR_NBE   2
#define CLN_HTR_MAT   3           //Elevation matrix
//=====================================================================================
//  DEFINE COLUMN INDICES FOR  COAST
//=====================================================================================
#define CLN_CST_QGT  0            // QGT key
#define CLN_CST_DET  1            // Detail tile key
#define CLN_CST_NBV  2            // Number of Vertices
#define CLN_CST_VTX  3            // Vertice blob
//=====================================================================================
//  DEFINE COLUMN INDICES FOR  TEXTURE
//=====================================================================================
#define CLN_TEX_KEY  0            // Texture Name (Key)
#define CLN_TEX_WID  1            // Width
#define CLN_TEX_HTR  2            // Height
#define CLN_TEX_IMG  3            // Texture image
//-------------------------------------------------------------------------------------
#define CLN_TXT_KEY  0            // Texture KEY
#define CLN_TXT_WID  1            // Texture Width
#define CLN_TXT_HTR  2            // Texture Height
#define CLN_TXT_IMG  3            // BLOB
//=====================================================================================
//  DEFINE COLUMN INDICES FOR  3Dmodel
//=====================================================================================
#define CLN_MOD_NAM 0             // Model file name
#define CLN_MOD_TSP 1             // Transparent indicator
#define CLN_MOD_TYP 2             // Type of model
#define CLN_MOD_TXN 3             // Texture name
#define CLN_MOD_TOP 4             // TOP Feet
#define CLN_MOD_BOT 5             // Bottom feet
#define CLN_MOD_LOD 6             // Level of Detail
#define CLN_MOD_NVT 7             // Number of vertices
#define CLN_MOD_NIX 8             // Number of indices
#define CLN_MOD_VTX 9             // Vertex table
#define CLN_MOD_NTB 10             // Normal table
#define CLN_MOD_TTB 11            // Texture table
#define CLN_MOD_ITB 12            // Indices table
//------------------------------------------------------------
#define CLN_NTX_NAM 0             // Texture name
#define CLN_NTX_WID 1             // Width
#define CLN_NTX_HTR 2             // Height
#define CLN_NTX_TEX 3             // Texture bytes
//------------------------------------------------------------
#define CLN_LST_TYP 0             // Type
#define CLN_LST_NIT 1             // Has night texture
#define CLN_LST_DSC 2             // Description
//=====================================================================================
//  DEFINE COLUMN INDICES FOR  PAVEMENT
//=====================================================================================
#define CLN_PAV_KEY 0             // Airport key
#define CLN_PAV_TYP 1             // Pavement type
#define CLN_PAV_USE 2             // AIrport usage
#define CLN_PAV_NBV 3             // Vertex number
#define CLN_PAV_TAB 4             // Vertex table
//=====================================================================================
//  DEFINE COLUMN INDICES FOR  LIGTH
//=====================================================================================
#define CLN_LIT_KEY 0             // Airport key
#define CLN_LIT_TYP 1             // Light type
#define CLN_LIT_TEX 2             // Texture number
#define CLN_LIT_COL 3             // Light color
#define CLN_LIT_PM1 4             // Parameter 1
#define CLN_LIT_PM2 5             // Parameter 2
#define CLN_LIT_NBS 6             // Number of spots
#define CLN_LIT_TAB 7             // Spot table
//=====================================================================================
//  DEFINE COLUMN INDICES FOR  3DOBJECT LIGHTS
//=====================================================================================
#define CLN_LTO_XKP 0             // Mother X Key	
#define CLN_LTO_YKP 1             // Mother Y key
#define CLN_LTO_DAY 2             // Day model
#define CLN_LTO_MOD 3             // Light mode
#define CLN_LTO_COL 4             // Color number
#define CLN_LTO_NTX 5             // Texture number
#define CLN_LTO_RAD 6             // Halo radius
#define CLN_LTO_FRQ 7             // Light frequency (total in sec)
#define CLN_LTO_ONT 8             // Total on time
#define CLN_LTO_XOF 9             // X offset
#define CLN_LTO_YOF 10            // Y Offset
#define CLN_LTO_ZOF 11            // Z Offset
//=====================================================================================
//  DEFINE COLUMN INDICES FOR  3DOBJECT LIGHTS
//=====================================================================================
#define CLN_OBJ_QGT  0            // QGT X-Z number
#define CLN_OBJ_TYP	 1						// Object typ
#define CLN_OBJ_XKP  2            // X position
#define CLN_OBJ_YKP  3            // Y Position
#define CLN_OBJ_KND  4            // Object Kind
#define CLN_OBJ_FLG  5            // Object flag
#define CLN_OBJ_DAY  6            // Day model
#define CLN_OBJ_NIT  7            // Nigth model
//-----------------------------------------------
#define CLN_OBJ_LON  8            // Longitude
#define CLN_OBJ_LAT  9            // Latitude
#define CLN_OBJ_ORX 10            // X Orientation
#define CLN_OBJ_ORY 11            // Y Orientation
#define CLN_OBJ_ORZ 12            // Z Orientation
//------------------------------------------------
#define CLN_OBJ_FOB 13            // mother file
#define CLN_OBJ_NZB 14            // No Z Buffer
#define CLN_OBJ_NZU 15            // No depth test
#define CLN_OBJ_PM1 16            // Parameter 1
#define CLN_OBJ_PM2 17            // Parameter 2
//------------------------------------------------
#define CLN_OBJ_NAM 18            // Name
#define CLN_OBJ_DSC 19            // Descriptor
//=====================================================================================
//  DEFINE COLUMN INDICES FOR  ELEVATION IN TRN TABLE
//=====================================================================================
#define CLN_ETR_QGT		0						// QGT key
#define CLN_ETR_SUP		1						// SUper Tile No
#define CLN_ETR_DET		2						// Detail Tile No
#define CLN_ETR_FIL		3						// File name
#define CLN_ETR_POD   4						// POD NAME
#define CLN_ETR_FLG		5						// Flag
#define CLN_ETR_SUB		6						// Subdivision level
#define CLN_ETR_USR   7						// User texture indicator
#define CLN_ETR_WTR		8						// Water flag
#define CLN_ETR_NIT		9						// Night flag
#define CLN_ETR_TXN	 10						// Texture name
#define CLN_ETR_NBE	 11						// Number of elevations
#define CLN_ETR_ELV	 12						// Elevation array
//-----TRN DETAIL TILE --------------------------------------
#define CLN_TIL_SUP   1						// Super Tile No
#define CLN_TIL_DET		2						// Detail Tile
#define CLN_TIL_SUB		5						// Tile subdivision
#define CLN_TIL_NBE   6						// Number of elevations
#define CLN_TIL_ELV   7						// Elevation array
//-----PATCHE ELEVATION --------------------------------------
#define CLN_PCH_KEY		0
#define CLN_PCH_DET		1						// Detail tile
#define CLN_PCH_SUB		2						// Tile subdivision
#define CLN_PCH_NBE		3						// Number of elevation
#define CLN_PCH_ELV		4						// Elevation array
//=====================================================================================
#define TEX_MED_KEY (U_INT)(0 << 30)
#define TEX_HIG_KEY (U_INT)(1 << 30)
#define TEX_NIT_KEY (U_INT)(2 << 30)
//=====================================================================================
//  DEFINE TYPE OF ELEVATION
//=====================================================================================
#define ELV_DEFAULT 0
#define ELV_DETAIL  1
//=====================================================================================
//  Define a database
//=====================================================================================
struct SQL_DB {
		int				vers;								// Minimum version
		int				mode;								// Open mode
    char      path[MAX_PATH];     // path name
		char      name[64];						// File name
    sqlite3  *sqlOB;              // Object database
    char      exp;                // Export indicator
    char      opn;                // Open indicator
    char      use;                // Use database
    char      mgr;                // Thread only if no export
    char     *dbn;                // Database name
		//--- Constructor ------------------------------
		SQL_DB::SQL_DB()
		{	mode	= SQLITE_OPEN_READONLY;
			exp		= 0;
			opn		= 0;
			use		= 0;
			vers	= 0;
		}
		//----------------------------------------------
};
//=====================================================================================
//  DEFINE A SQL REQUEST
//=====================================================================================
struct SQL_REQ {
  char      txt[256];                 // Request statement
  char     *arg[16];                  // Pointer to parameters
};
//=====================================================================================
//  DEFINE A SQL REQUEST CODE
//=====================================================================================
#define SQL_APT_LINE 1
#define SQL_RWY_LINE 2
#define SQL_NAV_LINE 3
#define SQL_WPT_LINE 4
#define SQL_COM_LINE 5
#define SQL_STA_LINE 6
#define SQL_CTY_LINE 7
//=====================================================================================
//  Available call back functions
//=====================================================================================
void APTtoCache(CmHead   *apt,CTileCache *tc);
void OBJtoCache(CmHead   *obj,CTileCache *tc); 
void RWYtoCache(CRunway  *rwy,CTileCache *tc);
void ELVtoCache(REGION_REC &rg);
void ELVtoSlots(REGION_REC &rg);
//-------------------------------------------------------------------
typedef enum {
  SQL_MGR = 1,
  SQL_THR = 2,
} SqlEType;
//=====================================================================================
//  Common class for SQL MANAGEMENT
//=====================================================================================
class SqlOBJ {
protected:
  //---Type of manager ----------------------------------------------
  U_CHAR    sqlTYP;                       // Type of base
  U_CHAR    tr;                           // Trace indicator
	char      rfu1;													// Reserved
	char			rfu2;													// Reserved
	U_INT			count;
	C_QGT			*qgt;
	//------Attributes ------------------------------------------------
	sqlite3_stmt *stm;
	C_STile      *sup;
  //---Common attributes --------------------------------------------
  SQL_DB    genDBE;                       // Generic database
  //-----------------------------------------------------------------
  SQL_DB    wptDBE;                       // Waypoint database
  //-----------------------------------------------------------------
  SQL_DB    elvDBE;                       // Elevation database
  //-----------------------------------------------------------------
  SQL_DB    seaDBE;                       // Coast Database
  //-----------------------------------------------------------------
  SQL_DB    txyDBE;                       // Taxiway database
  //-----------------------------------------------------------------
  SQL_DB    modDBE;                       // 3D model Database
  //-----------------------------------------------------------------
  SQL_DB    texDBE;                       // Texture Database
  //-----------------------------------------------------------------
  SQL_DB    objDBE;                       // Object database
  //-----------------------------------------------------------------
	SQL_DB    t2dDBE;												// Texture 2D database
  //---Common methods -----------------------------------------------
public:
  SqlOBJ();
 ~SqlOBJ();
  //-----------------------------------------------------------------
  void          Init();
  void          OpenBases();
  int           Open(SQL_DB &db);
  void          WarnE(SQL_DB &db);
  void          Warn1(SQL_DB &db);
	int						ReadVersion(SQL_DB &db);
	void					ImportConfiguration(char *fn);
	//-----------------------------------------------------------------
  sqlite3_stmt *CompileREQ(char *req,SQL_DB &db);
  void          Abort(SQL_DB &db);
  //---Elevation management -----------------------------------------
  void          DecodeREG(sqlite3_stmt *stm,REGION_REC &reg);
  void          ReadElevation(REGION_REC &reg);
  void          GetQgtElevation(REGION_REC &reg,ElvFunCB *fun);
  void          DeleteElvDetail(U_INT key);
  void          DeleteElvRegion(U_INT key);
  void          DeleteElevation(U_INT key);
	//-----------------------------------------------------------------
  inline char   UseELV()			{return elvDBE.use;}
  inline bool   MainSQL()			{return (sqlTYP == SQL_MGR);}
};
//=====================================================================================
//  CLASS SQL MANAGER to handle data access in main THREAD
//=====================================================================================
class SqlMGR: public SqlOBJ {
  //---------ATTRIBUTE OBJECT ---------------------------------------------
  char     *query;                        // Current query
  char      nArg;                         // Argument number
  //-----------------------------------------------------------------
  int       low;                          // Low resolution
  int       med;                          // Medium resolution
  //-----------------------------------------------------------------
  int       wd;                           // Last texture width
  int       ht;                           // Last texture height
  //-----------------------------------------------------------------
  char      wptPATH [PATH_MAX];           // WayPoint path
  //-----------------------------------------------------------------
public:
  SqlMGR();
 ~SqlMGR();
  //-----------------------------------------------------------------------
  inline bool SQLgen()      {return (1 == genDBE.use );}
  inline bool SQLelv()      {return (1 == elvDBE.use );}
  inline bool SQLsea()      {return (1 == seaDBE.use );}
  inline bool SQLmod()      {return (1 == modDBE.use );}
  inline bool SQLtxy()      {return (1 == txyDBE.use );}
  inline bool SQLtex()      {return (1 == texDBE.use );}
  inline bool SQLobj()      {return (1 == objDBE.use );}
	inline bool SQLt2d()			{return (1 == t2dDBE.use );}
  //-----------------------------------------------------------------------
  CComLine    *GetComSlot(sqlite3_stmt *stm,CDataBaseREQ *req);
  CAptLine    *GetAptSlot(sqlite3_stmt *stm);
  CNavLine    *GetNavSlot(sqlite3_stmt *stm);
  CWptLine    *GetWptSlot(sqlite3_stmt *stm);
  CRwyLine    *GetRwySlot(sqlite3_stmt *stm);
  CCtyLine    *GetCtySlot(sqlite3_stmt *stm);
  CStaLine    *GetStaSlot(sqlite3_stmt *stm);
  CComLine    *GetIlsSlot(sqlite3_stmt *stm);
  //-----------------------------------------------------------------------
  CAirport    *DecodeAPT(sqlite3_stmt *stm, OTYPE obt);
  CRunway     *DecodeRWY(sqlite3_stmt *stm, OTYPE obt);
  CNavaid     *DecodeNAV(sqlite3_stmt *stm, OTYPE obt);
  CILS        *DecodeILS(sqlite3_stmt *stm, OTYPE obt);
  CCOM        *DecodeCOM(sqlite3_stmt *stm, OTYPE obt);
  CWPT        *DecodeWPT(sqlite3_stmt *stm, OTYPE obt);
  //----LOCALIZATION ROUTINES FOR CACHE MANAGEMENT  -----------------------
  void  GetNavByTile(int key,           HdrFunCB *fun,CTileCache *tc);
  void  GetAptByTile(int key,           HdrFunCB *fun,CTileCache *tc);
  void  GetComByTile(int key,           HdrFunCB *fun,CTileCache *tc);
  void  GetRwyForAirport(CAirport *apt, RwyFunCB *fun,CTileCache *tc);
  void  GetIlsForAirport(CAirport *apt, HdrFunCB *fun,CTileCache *tc);
  void  GetAllAirports(AptFunCB *fun);
  //-----------------------------------------------------------------------
  void  BuildARG(CDataBaseREQ *req);
  void  BuildSQL(CDataBaseREQ *req,char *query);
  //-----WINDOWS DIRECTORY ACCESS   ---------------------------------------
  void  GetAirportByIden (char *idn,CAirport **ptr);
  void  GetAirportByArg  (CDataBaseREQ *req);
  void  GetNavaidByArg   (CDataBaseREQ *req);
  void  GetWaypointByArg (CDataBaseREQ *req);
  void  GetAllCountries  (CDataBaseREQ *req);
  void  GetOneCountry    (CDataBaseREQ *req);
  void  GetStateByCountry(CDataBaseREQ *req);
  void  GetCountryName(TCacheMGR *tcm);
  //----DETAIL WINDOW ACCESS ----------------------------------------------
  void  ComFromThisAirport(CDataBaseREQ *req);
  void  APTByKey(CDataBaseREQ *req);
  void  NAVByKey(CDataBaseREQ *req);
  void  RWYbyAPTkey(CDataBaseREQ *req);
  void  ILSbyRWYkey(CDataBaseREQ *req, CRwyLine *lin);
  //----ACCESS FOR GPS KLN89-----------------------------------------------
  void  AirportRWYforGPS(CGPSrequest *req,CAirport *apt);
  void  AirportCOMforGPS(CGPSrequest *req,CAirport *apt);
  void  RunwayILSforGPS (CGPSrequest *req, char *akey,char *kend);
  //----MATCH FOR GPS -----------------------------------------------------
  sqlite3_stmt *CompileMatchGPS(CGPSrequest *req,char *tab);
  void     MatchAPTforGPS(CGPSrequest *req,int dir);
  void     MatchNAVforGPS(CGPSrequest *req,int dir);
  void     MatchWPTforGPS(CGPSrequest *req,int dir);
  void     GetOBJbyIdent (CGPSrequest *req, char *tab, Tag type);
  //----ACCESS FOR FLIGHT PLAN --------------------------------------------
  void     GetFlightPlanWPT(CWPoint* wpt);
  //----Elevation management ----------------------------------------------
  int      WriteElevationBlob  (REGION_REC &reg);
  void     WriteElevationRegion(REGION_REC &reg);
  void     WriteElevationRecord(REGION_REC &reg);
	void		 WriteElevationTRN(C_STile &sup,U_INT row);
	void		 WriteElevationDET(U_INT key,TRN_HDTL &hdl,int row);
  void     ELVtransaction();
  void     ELVcommit();
	int      WriteTRNname(char *fn);
  //----WRITING FOR COAST DATA ---------------------------------------------
  void     SEAtransaction();
  void     SEAcommit();
  void     WriteCoastRec(COAST_REC &cst);
  //----WRITING 3D model --------------------------------------------------
  bool     Check3DModel(char *name);
  bool     Check3DTexture(char *name);
  void     Write3Dmodel(char *name,C3DPart *prt);
  int      WriteM3DTexture(TEXT_INFO &inf);
	//----UPDATING 3D model -----------------------------------------------
	int			 DeleteM3DModel(char *fn);
	int			 RemoveM3DModel(char *name);
	int		   RemoveM3DTexture(sqlite3_stmt *stm);
  //----WRITING TAXIWAY DATA --------------------------------------------
  void     WritePavement(CPaveRWY *pave,char *key);
  void     WriteTaxiLigth(CBaseLITE *lite, char * key);
  int      DecodePAVE(CAptObject *apo);
  int      DecodeLITE(CAptObject *apo);
  //---- WRITING TEXTURE DATA -------------------------------------------
  int      WriteGenTexture(TEXT_INFO &inf,char *tab);
  int      WriteAnyTexture(TEXT_INFO &inf);
	void		 TEXcommit();
  void     GetTerraList(tgxFunCB *fun);
  void     GetTerraSQL(TCacheMGR *tcm);
  //-----3D WORLD OBJECT ------------------------------------------------
  void    WriteWOBJ(U_INT qgt,CWobj *obj,int row);
  void    Write3DLight(CWobj *obj,C3DLight *lit);
  void    WriteLightsFrom(CWobj *obj);
	int     WriteOBJname(char *fn);
	bool		FileInOBJ(char *fn);
	int 		SearchWOBJ(char *fn);
	bool		SearchPODinOBJ(char *pn);
  int     ReadWOBJ(C_QGT *qgt);
  void    ReadOBJLite(CWobj *obj,int xk,int yk);
	void		ReadOBJFile(CWobj *obj,int row);
  void    DecodeWOBJ(sqlite3_stmt *stm,CWobj *obj);
  void    Decode3DLight(sqlite3_stmt *stm,CWobj *obj);
	void		UpdateOBJzb(CWobj *obj);
  //---- READING TEXTURE ------------------------------------------------
  void     DecodeLinTexture(sqlite3_stmt *stm, CTgxLine *lin);
  GLubyte *GetGenTexture(TEXT_INFO &inf);
  GLubyte *GetAnyTexture(TEXT_INFO &inf);
  //--- SPECIFIC TRN ELEVATIONS -----------------------------------------
	bool	FileInELV(char *fn, U_INT *r);
	bool	SearchPODinTRN(char *pn);
	int		GetTRNElevations(C_QGT *qgt);
	int		DecodeTRNrow();
	int		GetTILElevations(C_QGT *qgt);
	int		DecodeDETrow();
	int   ReadPatches(C_QGT *qgt,ELV_PATCHE &p);
	int		WriteT2D(TEXT_INFO *tdf);
	bool	TileInBase(U_INT key);
	//--- WRITING PATCHE ELEVATIONS ---------------------------------------
	int		WritePatche (ELV_PATCHE &p);
	int		DeletePatche(ELV_PATCHE &p);
};
//=============================================================================
//  SQL THREAD
//  Support for Databases used by file thread
//=============================================================================
class SqlTHREAD: public SqlOBJ {
	//---- Attribute ---------------------------------------------------
	bool		go;						// Running indicator
  //-----Methods -----------------------------------------------------
public:
  SqlTHREAD();
 ~SqlTHREAD();
  //------------------------------------------------------------------
  inline bool SQLelv()  {return (elvDBE.use == 1);}
  inline bool SQLsea()  {return (seaDBE.use == 1);}
  inline bool SQLmod()  {return (modDBE.use == 1);}
  inline bool SQLtex()  {return (texDBE.use == 1);}
	//--- Execution control --------------------------------------------
	inline bool IsRuning()	{return go;}
	inline void Stop()			{go = false;}
  //---COAST METHODS -------------------------------------------------
  void  DecodeCST(sqlite3_stmt *stm,COAST_REC &cst);
  void  ReadCoast(COAST_REC &rec,C_CDT *cst);
  void  CopyData (COAST_REC &rec);
  //--3DMODEL --------------------------------------------------------
  bool  CheckM3DModel(char *name);
  int   GetM3DTexture(TEXT_INFO *inf);
  int   DecodeM3DPart(sqlite3_stmt *stm,C3Dmodel *modl);
  int   GetM3Dmodel(C3Dmodel *modl);
  //---Generic texture -----------------------------------------------
  GLubyte *GetGenTexture(TEXT_INFO &txd);
};

//=======END OF FILE ==================================================================
#endif // SQLMGR_H

