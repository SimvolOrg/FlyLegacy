/*
 * Database.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 * Copyright 2007 Jean Sabatier
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

/*! \file Database.h
 *  \brief Defines classes for application access to Fly! databases
 *
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include <pthread.h>
#include <map>

#include "FlyLegacy.h"
//============================================================================================
class CWPoint;
//============================================================================================
//
// Type field translation for some Fly databases 
//
std::string & DBAirportType(int iType);
std::string & DBNavaidType(int iType);
std::string & DBComType(int iType);
std::string & DBILSType(int iType);
//
// Usage field translation for some Fly databases
//
std::string & DBWaypointUsage(int iUsage);

void InitTypeDBTranslation(void);
void CleanupTypeDBTranslation(void);


//
// Common class definitions
//
// These classes are used commonly across several database modules.
//
// These classes are implemented in DBUtilities.cpp
//
class CDatabaseField {
public:
  // Constructor/Destructor
  CDatabaseField (void);

  // Public methods
  char            GetType (void);       ///< Return data type (char, float, int)
  unsigned long   GetLength (void);     ///< Return data field length
  void            GetString (char *s);  ///< Copy string value to supplied buffer
  double          GetDouble (void);     ///< Get a double float value
  long            GetInt (void);        ///< Get a long integer value
  void            Format (char *s);     ///< Format data item for output

public:
  char           type;
  unsigned long  length;
  union {
    char    charData[256];
    double  doubleData;
    long    intData;
  } data;
};


//
// Template related class definitions
//
// Each database has a template (.DBT) file that defines the physical structure
//   of the database records.  This is a data-driven approach that allows new
//   databases to be integrated into the application with minimal effort.
//
// These classes are implemented in DBTemplate.cpp
//
class CDatabaseTemplateItem {
public:
  // Constructor/Destructor
  CDatabaseTemplateItem (void);

  // Public methods

public:
  Tag            id;
  unsigned long  rawOffset;
  unsigned long  rawLength;
  char           type;
  char           name[256];
};

class CDatabaseTemplate {
public:
  // Constructor/Destructor
  ~CDatabaseTemplate (void);

  // Public methods
  void                   Load (const char* fileName);
  int                    GetNumItems (void);
  CDatabaseTemplateItem* GetItem (int i);
  CDatabaseTemplateItem* GetItemByTag (Tag tag);
  int                    GetOffset (Tag tag);
  int                    GetLength (Tag tag);
  void                   Dump (FILE *f);

protected:
  // Protected methods
  void                   Load (PODFILE *f);

protected:
  std::vector<CDatabaseTemplateItem*>    item;  ///< Vector of template items
};


//
// Index related class definitions
//
// Each database can have an arbitrary number of indices assigned to it.  Each index
//   is stored in an associated .DBI file.  The index header specifies the type of
//   key that the index is based on; key types do not necessarily map directly to
//   a field in the database; applications that use the database must know how to
//   construct key values in order to effectively use them.
//
// These classes are implemented in DBIndex.cpp
//

typedef std::multimap<std::string, unsigned long>   StringIndex;
typedef std::multimap<double, unsigned long>        DoubleIndex;
typedef std::multimap<long, unsigned long>          IntIndex;

class CDatabaseIndex {
public:
  // Constructor/destructor
  CDatabaseIndex (const char* dbiFilename);
  ~CDatabaseIndex (void);

  // Public functions
  int           GetNumItems (void);
  Tag           GetKeyId (void);
  char          GetKeyType (void);
  unsigned long GetKeyLength (void);
  unsigned long Search (const char* key);
  unsigned long Search (double key);
  unsigned long Search (long key);
  unsigned long SearchNext (void);
  void          Dump (FILE *f);

protected:
  void          Load (PODFILE *f);
  void          Load (const char *dbiFilename);

protected:
  Tag           keyId;
  unsigned char keyType;
  unsigned long keyLength;

  std::multimap<std::string, unsigned long> *stringIndex;
  std::multimap<double, unsigned long> *doubleIndex;
  std::multimap<long, unsigned long>   *intIndex;

  std::pair<StringIndex::iterator,StringIndex::iterator>  stringRange;
  std::pair<DoubleIndex::iterator,DoubleIndex::iterator>  doubleRange;
  std::pair<IntIndex::iterator,IntIndex::iterator>        intRange;

  StringIndex::iterator   stringIter;
  DoubleIndex::iterator   doubleIter;
  IntIndex::iterator      intIter;
};


//============================================================================
// Database related class definitions
//
// These classes are implemented in DBDatabase.cpp
//============================================================================
class CDatabaseRecord {
public:
  // Constructor/Destructor
  CDatabaseRecord (void);

  // Public methods
  void            SetSequence (unsigned long sequence);
  void            AddField (CDatabaseField & f);
  int             GetNumFields (void);
  CDatabaseField* GetField (unsigned int i);
  void            Reset();

protected:
  unsigned long                sequence;  ///< Unique sequence number of this record
  std::deque<CDatabaseField>  field;     ///< Vector of field values
};

typedef enum {
  DB_UNMOUNTED,
  DB_MOUNTED
} EDatabaseState;
//=====================================================================
//	Put forward declaration for managed objects here
//=====================================================================
class CmHead;					// Just forward declaration
class CNavaid;
class CWPT;
class CAirport;
class CILS;
class CRunway;
class CRadio;
class CBand;
class CCOM;
class COldFlightPlan;
class CWPoint;
class CFuiAptDetail;
class CFuiDirectory;
class CFuiVectorMap;
class CCountry;
class CFuiWindow;
class CState;
class CObstacle;
class TCacheMGR;                          // Terrain Cache manager
class CCtyLine;                           // Country line
class CStaLine;                           // State line
class CAptLine;                           // Airport line
class CNavLine;                           // Nav line
class CWptLine;                           // Waypoint line
class CComLine;                           // CCOM   line
class CRwyLine;                           // Runway line
class CRLP;                               // Runway light profile
class CLitSYS;                            // Light system
class CAptObject;                         // Airport object
class SqlMGR;                             // SQL Manager
class CSoundBUF;                          // Sound buffer
class CFuiFlightLog;                      // Window for flight plan
//====================================================================
//
// Data (DBD) related methods
//====================================================================
typedef struct {
  unsigned long  signature;
  unsigned long  unknown0;
  unsigned long  unknown1;
  unsigned long  checksum;
  unsigned long  nRecords;
  unsigned long  recLength;
  unsigned long  dummy[8];
} DBDHeader;
//===========================================================================
//	JSDEV*  complete database with some virtual decoder functions
//===========================================================================
//---------------------------------------------------------------------------
class CDatabase {
  private:
  // Database state----------------------------------------------
  EDatabaseState  state;
  PODFILE*        podfile;
  unsigned long   signature;
  unsigned long   nRecords;   
  unsigned long   recLength;

  CDatabaseTemplate                   dbt;    ///< Template
  std::multimap<Tag,CDatabaseIndex*>  dbi;    ///< Indices

  unsigned long       cacheOffset;            ///< DBD offset of last record read

  // Upper and lower bounds of the set of active indices for searching
  typedef std::multimap<Tag,CDatabaseIndex*>::iterator  dbiIter;
  dbiIter                                               dbiNext;
  std::pair<dbiIter,dbiIter>                            dbiPair;
  CDatabaseIndex*                                       dbiSearch;
  const char*                                           stringKey;
  double                                                doubleKey;
  long                                                   intKey;
  //---------------------------------------------------------------
protected:
  char*     buf;                     // Temporary raw record buffer
  //---------------------------------------------------------------
public:
  // Constructors
  CDatabase (const char* dbtFilename);
  ~CDatabase (void);
    
  // Template related methods
  CDatabaseTemplate* GetTemplate (void);

  // Index related methods
  void            AddIndex (const char *dbiFilename);
  unsigned int    GetNumIndexes (void);
  CDatabaseIndex* GetFirstIndex (Tag tag);
  CDatabaseIndex* GetNextIndex (void);

public:
  // Data import/export
  void      Mount (PODFILE *f);
  void      Mount (const char *dbdFilename);
  void      Unmount (void);
  void      Dump (FILE *f);
  bool      NotMounted()  {return (state != DB_MOUNTED);}
public:
  // Data access members
	virtual void	    DecodeRecord(U_LONG offset,CmHead *obj){};
  virtual void      DecodeRecord(char *rec,CSlot *slot){};
  virtual int       GetAccessKey(void)  {return 0;}
  //--------------------------------------------------------------------------
	bool              GetRecordByOffset (unsigned long offset, CDatabaseRecord & rec);
	bool              GetRecordByIndex (unsigned long index, CDatabaseRecord & rec);
	unsigned long     Search (Tag index, const char* key);
	unsigned long     Search (Tag index, double key);
	unsigned long     Search (Tag index, long key);
	unsigned long     SearchNext (void);
	void              GetRawRecord (unsigned long offset);
	bool              GetFieldByIndex(unsigned long index, 
                                    CDatabaseField & outField,
                                    unsigned int fieldPos);
  //--------Return record number given record offset ----------------------------
  inline  U_INT   GetRecordNo(U_LONG offset){return (offset - sizeof(DBDHeader)) / recLength;}
  //--------Return record offset given record number ----------------------------
  inline  U_LONG  RecordOffset(U_LONG index){return sizeof(DBDHeader) + (index * recLength);}
  //-------------------------------------------------------------------------------------------
  inline char *            GetBuffer()          {return buf;}
  inline unsigned long     GetRawRecordLength() {return recLength;}
  inline unsigned long     GetNumRecords()      {return nRecords; }
  inline unsigned long     GetNumFields ()      {return dbt.GetNumItems();}
  //--------------------------------------------------------------------------

protected:
  void            LoadTemplate (const char* dbtFilename);

                                             
};
//----------Option for label edit --------------------------------------------------
//#define VM_NAME 0x01
//#define VM_IDEN 0x02
//#define VM_FREQ 0x04
//==================================================================================
//  Database Request.  Purpose Acces serialization
//==================================================================================
class CDataBaseREQ: public CmHead {
friend class CDbCacheMgr;
friend class SqlMGR;
protected:
  //-----Attributes -----------------------------------------------------
  U_CHAR        Code;                           // Request code
  U_CHAR        Prev;                           // Previous code
  U_CHAR        filter;                         // Type of search
  U_CHAR        State;
  U_INT         Mask;                           // Filter for com
  char          Direct;                         // Direction
  long          offset;                         // Record offset
  CFuiWindow   *Wind;                           // Requesting window
  char         *aptkey;                         // Airport key
  char         *ctykey;                         // Country key
  char         *stakey;                         // State key
  char         *st1key;                         // Search criteria 1
  char         *st2key;                         // search criteria 2
  //--------------------------------------------------------------------
public:
  CDataBaseREQ(CFuiWindow *win,DBCODE req);
  CDataBaseREQ();
  void          SetCTY(char *cty);
  void          SetSTA(char *sta);
  void          SetST1(char *st1); 
  void          SetST2(char *st2);
  void          SetOFS(U_LONG of) {offset = of;}
  //---------------------------------------------------------------------
  inline  char  *GetOBJkey()                  {return aptkey;}
  inline  char  *GetCTYkey()                  {return ctykey;}
  inline  char  *GetSTAkey()                  {return stakey;}
  inline  char  *GetST1key()                  {return st1key;}
  inline  char  *GetST2key()                  {return st2key;}
  inline  U_LONG GetOffset()                  {return offset;}
  inline  DBCODE GetOrder()                   {return (DBCODE)Prev;}
  inline  void   PushCode()                   {Prev = Code; Code = NO_REQUEST;}
  //---------------------------------------------------------------------
  inline  bool   Match(U_INT m)               {return (m & Mask) != 0;}
  inline  void   SetAPT(char *k)              {aptkey = k;}
  inline  DBCODE GetReqCode()                 {return (DBCODE)Code;  }
  inline  void   SetReqCode(DBCODE cd)        {Code   = cd;}
  inline  void   SetReqMask(int m)            {Mask   = m;}
  inline  void   SetDirection(U_CHAR dr)      {Direct = dr;}
  inline  void   SetWindow(CFuiWindow *win)   {Wind   = win;}
  inline  void   ClearFilter()                {filter = 0;}
  inline  void   SelectNDB()                  {filter |= 0x10;}
  inline  void   SelectVOR()                  {filter |= 0x20;}
  inline  U_CHAR NeedCTY()                    {return (filter & 0x01);}
  inline  U_CHAR NeedSTA()                    {return (filter & 0x02);}
  inline  U_CHAR NeedST1()                    {return (filter & 0x04);}
  inline  U_CHAR NeedST2()                    {return (filter & 0x08);}
  inline  U_CHAR NeedNDB()                    {return (filter & 0x10);}
  inline  U_CHAR NeedVOR()                    {return (filter & 0x20);}
  inline  bool   IsNotForUS()                 {return (strcmp(ctykey,"US") != 0);}
  inline  int    HasCTY()                     {return (filter & 0x01);}
  inline  bool   EndOfReq()                   {return (Code == REQUEST_END);}
};
//==================================================================================
//  GPS DATABASE REQUEST
//
//==================================================================================
class CGPSrequest: public CDataBaseREQ
{ friend class CDbCacheMgr;
  friend class CK89gps;
  friend class SqlMGR;
private:
  //------------Attributes ----------------------------------------
  QTYPE         actQ;                       // Active queue
  U_CHAR        nSTACK;                     // Nearest Stack in use
  short         wptNo;                      // Waypoint number
//  short         wptAC;                      // Active waypoint
  CObjPtr       obj;                        // Object smart pointer
  ClQueue        rwyQ;                       // Runway Queue
  ClQueue        comQ;                       // COMM queue
  ClQueue        ilsQ;                       // ILS  queue
  ClQueue        wptQ;                       // Waypoint queue
  //-----------Methods ---------------------------------------------
public:
  ~CGPSrequest();
  void          Clean();
  void          GetAPT();
  void          GetVOR();
  void          GetNDB();
  void          GetWPT();
  void          GetRWY();
  void          GetCOM();
  void          GetILS();
};
//==================================================================================
//	Country database
//==================================================================================
class CDatabaseCTY : public CDatabase {
public:
	CDatabaseCTY(const char* dbtFilename);
	//-------define decoder functions ----------------------------
	void DecodeRecord(CCountry *obj);
  void DecodeRecord(CSlot *slot);
};
//==================================================================================
//	State database
//==================================================================================
class CDatabaseSTA : public CDatabase {
public:
	CDatabaseSTA(const char* dbtFilename);
	//-------define decoder functions ----------------------------
	void DecodeRecord(CState *obj);
  void DecodeRecord(CStaLine *slot);
};
//==================================================================================
//	Navaid database
//==================================================================================
class CDatabaseNAV : public CDatabase {
public:
	CDatabaseNAV(const char* dbtFilename);
	//-------define decoder functions ----------------------------
	void DecodeRecord(U_LONG offset,CmHead *obj);
  void DecodeRecord(CNavLine *slot);
};
//==================================================================================
//	AIRPORT database
//==================================================================================
class CDatabaseAPT : public CDatabase {
public:
	CDatabaseAPT(const char* dbtFilename);
	//-------define decoder functions ----------------------------
	void DecodeRecord(U_LONG offset,CmHead *obj);
  void DecodeRecord(CAptLine *slot);
};

//==================================================================================
//	ILS database
//==================================================================================
class CDatabaseILS : public CDatabase {
public:
	CDatabaseILS(const char* dbtFilename);
	//-------define decoder functions ----------------------------
	void DecodeRecord(U_LONG offset,CmHead *obj);
  void DecodeRecord(CComLine *slot);
};
//==================================================================================
//	Runway database
//==================================================================================
class CDatabaseRWY : public CDatabase {
public:
	CDatabaseRWY(const char* dbtFilename);
	//-------define decoder functions ----------------------------
	void    DecodeRecord(U_LONG offset,CmHead *obj);
  void    DecodeRecord(CRwyLine *slot);
  inline  int  GetAccessKey(void) {return 'nltl'; }
};
//==================================================================================
//	COMM database
//==================================================================================
class CDatabaseCOM : public CDatabase {
public:
	CDatabaseCOM(const char* dbtFilename);
	//-------define decoder functions ----------------------------
	void    DecodeRecord(U_LONG offset,CmHead *obj);
  void    DecodeRecord(CComLine *slot);
};
//==================================================================================
//	Waypoint database
//==================================================================================
class CDatabaseWPT : public CDatabase {
public:
	CDatabaseWPT(const char* dbtFilename);
	//-------define decoder functions ----------------------------
	void DecodeRecord(U_LONG offset,CmHead *obj);
  void DecodeRecord(CWptLine *slot);
  
};
//=================================================================================
//  Obstuction Database
//=================================================================================
class CDatabaseOBS : public CDatabase {
public:
  CDatabaseOBS(const char* dbtFilename);
  //-------define decoder functions ----------------------------
	void DecodeRecord(U_LONG offset,CmHead *obj);

};
//=================================================================================
//
// DBUtilities.cpp
//
//void CorrectEndian (void *pData, int nBytes);
void ReadLong (PODFILE *f, long *i);
void ReadULong (PODFILE *f, unsigned long *i);
void ReadUShort (PODFILE *f, unsigned short *i);
void ReadUChar (PODFILE *f, unsigned char *i);
void ReadDouble (PODFILE *f, double *i);
void ReadFloat (PODFILE *f, double *i);

char * TabNextField (char *s, char *szField);
char * TabSkipFields (char *s, int nFields);

//=================================================================================
//	JSDEV* LOCAL CACHE DATABASE MANAGEMENT
//	This database acts as a memory cache for all objects surrounding the aircraft
//	like navaids, ndb, ILS, airports, etc
//	It is kept up to data with the aircraft motion over time
//=================================================================================



//=====================================================================
//	CTileCache is used to remember all objects introduced
//	from a given tile
//=====================================================================
class CDbCacheMgr;			                    // Forward declaration
//=====================================================================
class CTileCache : public CmHead {
	friend class CDbCacheMgr;
  friend class SqlMGR;
  enum ACTION { INS_NAV,                    // Insert NAVs
                INS_APT,                    // Insert Airports
                INS_RWY,                    // Insert runways
                INS_ILS,                    // Insert ILS
                INS_COM,                    // Insert COMM
                INS_WPT,                    // Insert waypoint
                INS_OBS,                    // Insert obstruction
                POP_TIL,                    // Pop tile and TRACE Tile total
                REMOVE,                     // Remove all queues
                TC_READY,                   // Tile is complete
              };
	//---------Attributes ------------------------------
protected:
  U_INT    tr;                             // Trace indicator
  int      NbObj;                          // Number of allocated objects
	U_INT		 gKey;						               // Globe Tile key
	U_INT		 NoFrame;						             // Frame stamp
  ACTION   Action;                         // Requested action
	ClQueue	 qList[QDIM];					           // List of queue header
	ClQueue *qHead[QDIM];					           // List of queue adresse
	//---------------------------------------------------
public:
			CTileCache(OTYPE qo,QTYPE qa);
			void PutInTile(CmHead *obj);	                // Put object in Tile Queue
			CmHead *PopObj(QTYPE qx);				            // Pop first object from queue
			int  FreeQueue(QTYPE qx);					              // Free a given queue
			void FreeAllQueues();						                // Free all queues
			void Trace(char *op,U_INT FrNo,U_INT key);
  //----------------------------------------------------
	inline	void  Stamp(U_INT FrNo)	    {NoFrame = FrNo;}	  // Stamp tile cache
	inline  void  SetKey(U_INT kxy)	    {gKey    = kxy;}	  // Store key
  inline  void  SetReady()            {Action  = TC_READY;}
	inline	bool  HasSee(U_INT FrNo)    {return (NoFrame == FrNo); }
	inline  U_INT GetKey()			        {return  gKey; }
  inline  bool  IsEmpty()             {return (0 == NbObj);}
  inline  bool  HasObject()           {return (0 != NbObj);}
  inline  bool  IsComplete()          {return (Action == TC_READY);}

};

//=====================================================================
//
//	CDbCacheMgr manages the whole thing
//  NOTE:   Magnetic deviation is the average deviation of all objects 
//          in cache. This is a substitute for the magnetic model that 
//          is no more working
//
//=====================================================================
//=====================================================================
class CDbCacheMgr {
  //-----------Stack for sorted waypoints --------------------------
  typedef struct {  U_CHAR    nWPT;          // Number of waypoints
                    U_CHAR    xWPT;          // Current waypoint
                    CObjPtr   List[8];       // Stack of pointers
  } WPT_STACK;
  //---------CallBack to add object to Cache-----------------------------
  typedef void (CDbCacheMgr::*dbFunCB)(CmHead *obj,CTileCache *tc); 
	//---------Attributes -------------------------------------------------
protected:
  U_INT     tr;                           // Trace indicator
  char      uWPT;                         // Use Waypoint indicator
  char      uOBS;                         // Use Obstacle indicator
  char      gSQL;                         // Use generic SQL
  SPosition aPos;                         // Aircraft position
	U_INT	  RefKey;								          // Reference Key (Aircraft tile)
  U_INT   NewKey;                         // Futur reference Key
	U_INT	  ctKey;								          // Current tile key
	U_INT	  cFrame;								          // Current Frame
  //-----Edit action slots ----------------------------------------------
  U_CHAR  cycle;                          // Cycle for cache refresh
  U_CHAR   stab;                          // Stable state
  //-----Lists of objects in cache --------------------------------------
  ClQueue    ActQ;                        // Action Queue
	ClQueue  aList[QDIM];						        // All cache queues
	ClQueue *aHead[QDIM];						        // address of each active queue
  int      count[QDIM];                   // Object count
  dbFunCB        cbFUN;                   // Call back function
  //-------Megnetic deviation -------------------------------------------
  double         totDEV;                  // Deviation (true north)
  double         magDEV;                  // Average deviation
  int            nbrDEV;                  // Number of object
  //-------Registered windows -------------------------------------------
  CFuiFlightLog *wLOG;                    // Windows NavLog
  CFuiDirectory *wDIR;                    // Window directory
  CFuiVectorMap *wMAP;                    // Vector Map
  //-----Access requests ------------------------------------------------
  ClQueue        reqQ;                    // Queue of requests
  ClQueue        prmQ;                    // Permanent request
  CGPSrequest    rGPS;                    // GPS request
  //--------------Statistical data   ------------------------------------
  U_LONG         total;                   // Total memory used
  U_LONG         oSize[QDIM];             // Object dimension
  //-------------Stack of sorted waypoints ------------------------------
  WPT_STACK      objSTK;                  // Sorted Object stack
  //---------------------------------------------------------------------
public:	
	 CDbCacheMgr();
	~CDbCacheMgr();
  //---------------------------------------------------------------------
  void        TimeSlice(float dT,U_INT FrNo);		// Cache update
  void        GetRange(U_INT cz,U_SHORT &up,U_SHORT &dn);
  void        RefreshCache(U_INT FrNo);
  bool        IsaNewKey(U_INT key);
  bool        IsaActKey(U_INT key);
  void        DispatchAction(CTileCache *tc);   // Action dispatcher
  int         ExecuteAction(CTileCache *tc);    // Execute one action
  void        OrderOBJ(QTYPE qx,WPT_STACK *stk,char *idn,int k);
  CmHead  *FindNearestOBJ(CmHead *obj);
  CmHead  *FindFirstNearest(QTYPE qx);
  //----------------------------------------------------
  void  IncDeviation(float dev);
  void  DecDeviation(float dev);
  //----------------------------------------------------
  void        SetNearestVOR();
  void        SetNextNRSobj();
  void        SetPrevNRSobj();
	//----------CALL BACK --------------------------------
  void        ObjToCache(CmHead   *obj,CTileCache *tc);
  void        RwyToCache(CRunway  *rwy,CTileCache *tc);
  void        NavToCache(CmHead   *obj,CTileCache *tc);
  void        AptToCache(CmHead   *obj,CTileCache *tc);
  void        ComToCache(CmHead   *obj,CTileCache *tc);
  //----------------------------------------------------
  void        GetStats(CFuiCanva *cnv);
  //----------------------------------------------------
	friend class CmHead;
  friend class CTileCache;
protected:
      void      Statistic();
  		void      PopTile(char opt);				                  // Pop first tile from action queue
      void      Recycle(CmHead *obj);                       // Recycle object
			int	      PutInCache(CmHead *obj);		                // Put in cache queue
			void	    DetachFromCache(CmHead *obj);	              // Detach from local queue
			CmHead *AllocateObj(QTYPE qx);					              // Allocate object
			U_INT	    MakeKey(int gtx,int gty);		                // Make tile key
			void	    NewTileCache(U_INT key);		                // Create a new tile cache
      bool      ValidILS(CILS *obj);
  //---- Remove methods ---------------------------------
      void  RemoveTile(CTileCache* tc);
	//---- Access methods----------------------------------
			void	    Populate(CTileCache *tc,CDatabase *db,QTYPE qx);
      void      IlsFromPOD(CTileCache *tc,CDatabase *db);
      void      RunwayFromAirport(CTileCache *tc);
      void      IlsFromSQL(CTileCache *tc);
      void      RwyFromThisAirport(CDataBaseREQ *req);
      void      RwyFromThisAirport(CTileCache *tc,  CAirport *apt);
      void      ComFromThisAirport(CDataBaseREQ *req);
      void      IlsFromThisRunway (CDataBaseREQ *req, char* ikey);
      void      IlsFromThisRunway (CDataBaseREQ *req,CRwyLine *slot);
      bool      LinkILStoRunway(CmHead *obj, CTileCache *tc);
      CAirport *FindAPTforILS(char *kapt);
  //----GPS ----------------------------------------------
      void      AirportRWYforGPS(CGPSrequest *req,CAirport *apt);
      void      AirportCOMforGPS(CGPSrequest *req,CAirport *apt);
      void      RunwayILSforGPS (CGPSrequest *req, char *akey,char *kend);
  //--- Utilities ----------------------------------------
public:
      U_LONG    GetDBcountryNBrecords();
public:
      //---------Flight plan management --------------------
      void        PopulateNode(CWPoint *wpt);
      void        GetFlightPlanWPT(CWPoint* wp);
      float       GetFlatDistance(CmHead *obj);
      void        GetFeetDistance(int *dx,int *dy,SPosition org,SPosition des);
      CNavaid*    GetNavaidByNameAndKey(char *name,char *key);
      CAirport*   GetAirportByName(char *name);
      CWPT*  GetWaypointByKey(char *key);
      //---------Interface to radio components-----------------
			CNavaid*	GetTunedNAV(CNavaid *pn,U_INT FrNo,float freq);
			CNavaid*	GetTunedNDB(CNavaid *pn,U_INT FrNo,float freq);
 			CILS*	    GetTunedILS(CILS    *pi,U_INT FrNo,float freq);
      CCOM*     GetTunedCOM(CCOM    *pn,U_INT FrNo,float freq);
      //--------OBSTRUCTION MANAGEMENT ------------------------
      CObstacle *FindOBSbyDistance(CObstacle *old,float radius);
      //--------VOR MANAGEMENT --------------------------------
      CNavaid*  FindVORbyDistance(CNavaid *old,float radius);
      CILS*     BestILS(CILS* ils1,CILS* ils2);
      CILS*     GoodILS(CILS* ils1,CILS* ils2);
      CNavaid*  FindNDBbyDistance(CNavaid *old,float radius);
      CCOM*     FindCOM(U_INT FrNo,float freq);
      CNavaid*	FindVOR(U_INT FrNo,float freq);
      CILS*	    FindILS(U_INT FrNo,float freq);
      CNavaid*	FindNDB(U_INT FrNo,float freq);
      void      ReleaseNDB(CNavaid *ndb);
      void      ReleaseVOR(CNavaid *vor);
      //----For Airport -----------------------------------------
      void      SetAirportIdent(CCOM *com);
      CAirport *FindAPTbyDistance(CAirport *old,float radius);
      //----------------------------------------------------------
      inline    short       GetRelativePos(short vp) {return (vp < 128)?(127 - vp):(vp - 128);}
      inline    SPosition  *GetPlanePos() {return &aPos;}
      inline    int         GetTrace()    {return tr;}
      inline    bool        NotSynch(U_INT f)   {return (f != cFrame);}
      inline    U_CHAR      NotStable()         {return (stab == 0);}
      //------------------------------------------------------------------------------------
      U_SHORT     GetNavType(U_SHORT type);
      char*       GetObjCategoryName(CmHead *obj);
      //--------------AIRPORT DETAIL METHODS-------------------------
      void        ComputeDistance(CRwyLine *slot,SPosition org);
      void        MakeILSkey(char *edt,char *akey,char *end);
      //--------------------------------------------------------------
      void        GetCountryName(TCacheMGR *tcm);
      void        GetAllCountries(CDataBaseREQ *req);
      void        GetAirportByArg(CDataBaseREQ *req);
      void        GetNavaidByArg(CDataBaseREQ *req);
      void        GetWaypointByArg(CDataBaseREQ *req);
      void        GetStateByCountry(CDataBaseREQ *req);
      void        APTByOffset(CDataBaseREQ *req);
      void        APTByOffset(U_LONG offset,CAirport *apt);
      void        NDBByOffset(CDataBaseREQ *req);
      void        NAVByOffset(CDataBaseREQ *req);
      bool        FilterByString(char *abcd,char *name);
      bool        FilterAirport (char *rec,CDataBaseREQ *req);
      bool        FilterNavaid  (char *rec,CDataBaseREQ *req);
      bool        FilterWaypoint(char *rec,CDataBaseREQ *req);
      bool        FilterCountry(CSlot *slot,CDataBaseREQ *rep);
      //-----------Request management -------------------------------
      void        PostRequest (CDataBaseREQ *req);
      void        ExecuteREQ();
      //----------Immediate requests --------------------------------
      void        FirstAirportByIdent(char *iden,CAirport **ptr);
      void        AirportByIdentFromPOD(char *iden,CAirport **ptr);
      //-----------GPS management -----------------------------------
      void        TrackGPS();
      void        PostGPSRequest(CGPSrequest *req);
      void        GetAPTbyIdent();
      void        GetVORbyIdent();
      void        GetNDBbyIdent();
      void        GetWPTbyIdent();
      CWPT       *MatchWPTforGPS(CGPSrequest *req,int dir);
      CAirport   *MatchAPTforGPS(CGPSrequest *req,int dir);
      CNavaid    *MatchNAVforGPS(CGPSrequest *req,int dir);
      //-----------Window Management -------------------------------
      void        RegisterLOGwindow(CFuiFlightLog *win) {wLOG = win;}
      void        RegisterDIRwindow(CFuiDirectory *win) {wDIR = win;}
      void        RegisterMAPwindow(CFuiVectorMap *win) {wMAP = win;}
      CFuiFlightLog *GetLOGwindow()   {return wLOG;}
      CFuiDirectory *GetDIRwindow()   {return wDIR;}
      CGPSrequest   *GetGPSrequest()  {return &rGPS;}
 };
//=====================================================================
//  COM class
//  NOTE: As cfr2 to cfr5 are not used until now,
//  they are not decoded to save memory.  If needed modify DecodeRecord
//=====================================================================
class	CCOM : public CmHead {
	//-------------COMM attributes --from Disk database -------------
	friend class CDbCacheMgr;
  friend class SqlMGR;
	friend class CDatabaseCOM;
  //---------------------------------------------------------------
  U_INT recNo;                        // Record number
  char  ckey[10];                     // Key
  char  capt[10];                     // Airport key
  char  cnam[20];                     // Name
  int   ctyp;                         // Type
  float cfr1;                         // Frequency 1
  float cfr2;                         // Frequency 2
  float cfr3;                         // Frequency 3
  float cfr4;                         // Frequency 4
  float cfr5;                         // Frequency 5
  SPosition pos;                      // Position
  //----------------------------------------------------------------
  CObjPtr apOBJ;                      // related airport
  float nmiles;                       // Distance to aircarft
  RD_COM  comInx;                     // Com Index
  //----------------------------------------------------------------
protected:
  void  SetAttributes();
public:
	CCOM(OTYPE qo,QTYPE qa);
  void  Trace(char *op,U_INT FrNo,U_INT key);
  CCOM *IsThisComOK(float hz);
  void  Refresh(U_INT FrNo);
  bool  IsSelected(float hz);
  bool  IsInRange(void);
  void  WriteCVS(U_INT gx,U_INT gz,char *sep,SStream &st);
  //----------------------------------------------------------------
  inline  char      *GetApKey()              {return capt;}
  inline  CAirport  *GetAirport()            {return (CAirport*)apOBJ.Pointer();}
  //----------------------------------------------------------------
  inline  int  IsDEL() {return (ctyp & COMM_CLEARANCEDELIVERY);}
  inline  int  IsATI() {return (ctyp & COMM_ATIS);}
  //-----------------------------------------------------------------
  inline  float   GetFrequency(){return cfr1;}
  inline  U_SHORT GetComType()  {return (U_SHORT)ctyp;}
  inline  U_SHORT GetComIndex() {return (U_SHORT)comInx;}
  //-----------------------------------------------------------------
  inline  void    SetRecNo(U_INT No)      { recNo = No;}
  inline  U_INT   GetRecNo()              { return recNo;}
};
//=====================================================================
//	NAVAID class  
//=====================================================================
class	CNavaid : public CmHead {
	//-------------NAVAID attributes --from Disk database -------------
	friend class CDbCacheMgr;
	friend class CDatabaseNAV;
  friend class SqlMGR;
	friend void DecodeNavaidRecord (char* rec, CNavaid *navaid);
  //-----------------------------------------------------------------
protected:
  U_INT recNo;                        // Record number
  //------------------------------------------------------------
  char  xType;                        // Navaid type
  //-----attributes from database ------------------------------
  char  nkey[10];
 	char  naid[5];
	char	name[40];
 	float   freq;
  float   mDev;
  float   nsvr;
  char  ncty[3];
  char  nsta[3];
	U_SHORT	type;
	U_SHORT	ncla;
	U_SHORT usge;
	SPosition pos;
  float   npow;
	float   rang;
  //----GQT  key -----------------------------------------------
  float  pDis;                          // Plane distance (squared)
  //----Additional attributes for various purposes -------------
	short xobs;														// OBS direction
	float	radial;                         // Aircraft radial in °
	float	nmiles;                         // Distance to VOR/NDB
  float dsfeet;                         // Distance in feet
  float vdzRad;                         // Vertical dead zone radius
  char  efrq[16];                       // Edited frequency
	//------------------------------------------------------------
public:
	CNavaid(OTYPE qo,QTYPE qa);
  //------Virtual functions ----------------------------
	inline short      GetRefDirection()	{return xobs;}
  inline float      GetRadial()	      {return radial;}
	inline float      GetNmiles()	      {return nmiles;}
  inline char*      GetIdent()        {return naid; }
  inline char*      GetName()         {return name;}
  inline char*      GetCountry()      {return ncty;}
  inline char*      GetEFreq()        {return efrq;}
  inline float      GetLatitude()     {return pos.lat;}
  inline float      GetLongitude()    {return pos.lon;}
  inline float      GetElevation()    {return pos.alt;}
  inline float      GetFrequency()    {return freq;}
  //------Generic functions ----------------------------
	void		Trace(char *op,U_INT FrNo,U_INT key);
  inline U_SHORT    GetNavType()      {return xType;}
	inline int        GetType()		      {return type; }
  inline char*      GetDbKey()        {return nkey;}
  inline SPosition  GetPosition()     {return pos; }
  inline SPosition *ObjPosition()     {return &pos;}
  inline float      GetMagDev()       {return mDev; }
  inline int        GetIndexType()    {return int(xType);}
  inline bool       IsTuned()         {return (vdzRad < dsfeet);}
  inline bool       IsNDB()           {return (0 != (type & NAVAID_TYPE_NDB));}
  inline bool       IsVOR()           {return (0 == (type & NAVAID_TYPE_NDB));}
  inline bool       IsInRange()       {return (rang >= nmiles);	}
  inline bool		    IsNotInRange()    {return !(IsInRange());	}
  //-------------------------------------------------------
	inline  void      SetOBS(short d)			{xobs	  = d;}
  inline  void      SetRecNo(U_INT No)  {recNo  = No;}
  inline  void      SetPDIS(float d)    {pDis   = d;}
  inline  U_INT     GetRecNo()          {return recNo;}
  inline  float     GetPDIS()           {return pDis;}
  inline  float     GetFeetDistance()   {return dsfeet;}
	inline  double	  Sensibility()				{return 10;}
	inline	U_CHAR		SignalType()				{return SIGNAL_VOR;}
  //------------------------------------------------------
	void	    Refresh(U_INT FrNo);							// Update navaid
  void      WriteCVS(U_INT gx,U_INT gz,char *sep,SStream &st);
  //-------------------------------------------------------
protected:
  void      SetAttributes();
  void      SetNavIndex();
	CNavaid*	IsThisNavOK(float freq);
	CNavaid*	IsThisNDBOK(float freq);
	bool		  IsSelected(float freq);
};
//=====================================================================
//	CWPT class  for intersections
//=====================================================================
class CWPT : public CmHead {
	//-------------ILS attributes --from Disk database -------------
	friend class CDbCacheMgr;
  friend class SqlMGR;
	friend class CDatabaseWPT;
protected:
  U_INT     recNo;                        // Record number
  char      widn[5];                      // ident = first char from name
	Tag       user;													// User TAG
  //--------From database file -----------------------------------
  char      wkey[10];                     // Unique key
  char      wnam[26];                     // Wpt name
  char      wcty[3];                      // Country
  char      wsta[3];                      // State
  char      wloc;                         // At VOR location
  U_SHORT   wtyp;                         // Type
  U_SHORT   wuse;                         // Usage
  SPosition wpos;                         // positon lat long alti
	//---------------------------------------------------------------
	CWPoint  *node;													// Associated node
  float     wmag;                         // Magnetic dev
  float     wbrg;                         // Bearing to NAV
  float     wdis;                         // Distance to NAV
  char      wnav[10];                     // NAV key
  //--------Additional data  --------------------------------------
  float	    radial;                       // Aircraft radial in °
	float	    nmiles;                       // Distance to VOR/NDB
  float			dsfeet;                       // Distance in feet
  //---------------------------------------------------------------
protected:
  void  SetAttributes();
public:
  CWPT(OTYPE qo,QTYPE qa);           // Constructor
	void			Init(char *idn,SPosition *pos);
  void			SetPosition(SPosition p);
  void      WriteCVS(U_INT gx,U_INT gz,char *sep,SStream &st);
  //--------Virtual functions ---------------------------
  inline float      GetRadial()	      {return radial;}
	inline float      GetNmiles()	      {return nmiles;}
  inline char*      GetIdent()        {return widn; }
  inline char*      GetName()         {return wnam;}
  //--------Generic functions ----------------------------
	inline Tag				GetUser()					{return user;}
  inline int        GetType()		      {return wtyp; }
  inline char*      GetDbKey()        {return wkey;}
  inline char*      GetCountry()      {return wcty;}
  inline char*      GetState()        {return wsta;}
  inline SPosition  GetPosition()     {return wpos;}
  inline SPosition *ObjPosition()     {return &wpos;}
  inline float      GetMagDev()       {return wmag; }
  inline float      GetLatitude()     {return wpos.lat;}
  inline float      GetLongitude()    {return wpos.lon;}
  inline float      GetElevation()    {return wpos.alt;}
	inline float      GetDistance()			{return wdis;}
	inline CWPoint   *GetNode()					{return node;}
	//------------------------------------------------------
	inline	U_CHAR		SignalType()			{return SIGNAL_WPT;}
	inline  short			GetRefDirection() {return short(radial);}
	inline  float     GetFeetDistance() {return dsfeet;}
  //------------------------------------------------------
	inline void				SetNOD(CWPoint *w){node = w;}
  inline void       SetKey(char *k)   {strncpy(wkey,k,10);}
  inline void       SetIDN(char *k)   {strncpy(widn,k, 5);}
  inline void       SetNAM(char *n)   {strncpy(wnam,n,26);}
  inline void       SetCTY(char *c)   {strncpy(wcty,c, 3);}
  inline void       SetSTA(char *s)   {strncpy(wsta,s, 3);}
  inline void       SetLOC(int n)     {wloc = char(n);}
  inline void       SetTYP(int n)     {wtyp = short(n);}
  inline void       SetUSE(int n)     {wuse = short(n);}
  inline void       SetMGD(float f)   {wmag = f;}
  inline void       SetBRG(float f)   {wbrg = f;}
	inline void       SetDIS(float d)		{wdis = d;}
  //------------------------------------------------------
  inline  void      SetRecNo(U_INT No)  { recNo = No;}
  inline  U_INT     GetRecNo()          { return recNo;}
  //------------------------------------------------------
  void    Refresh(U_INT FrNo);
  void		Trace(char *op,U_INT FrNo,U_INT key);
};
//========================================================================
//  BEACON MARKERS
//========================================================================
typedef struct {
  double hmax;                           // Maximum height
  double hght;                           // expected height
  double dist;                           // Distance to threshold
  double tang;                           // Half Cone tan
} B_MARK;
//----------------------------------------------------------------
//  BEACON RESULT
//----------------------------------------------------------------
class CBeaconMark{
  //--------------------------------------------------------
public:
  char   ok;                            // Resulting
  char   on;                            // Flash on
  char   ch;                            // State change
  char   rfu;                           // Reserved
  double cone;                          // Cone radius
  double ydev;                          // delta Distance
  double xdev;                          // Deviation horizontal
  double timr;                          // Timer
  double endt;                          // Timer end
  double blnk;                          // Blink rate
  //----Sound Buffer ----------------------------------------
  CSoundBUF *sbuf;
  //---------------------------------------------------------
  CBeaconMark();
 ~CBeaconMark();
  void  Flash(float dT);
  char  Set(char s);
  //---------------------------------------------------------
  inline char State() {return (ok & on);}
}; 
//=====================================================================
//	ILS Type
//=====================================================================
class CRunway;
//----------------------------------------------------------------
class	CILS : public CmHead {
	//-------------ILS attributes --from Disk database -------------
	friend class CDbCacheMgr;
  friend class SqlMGR;
	friend class CDatabaseILS;
protected:
  U_INT   recNo;                          // Record number
  char    ikey[10];                       // ILS key
  char    iapt[10];                       // Airport ID
  char    irwy[4];                        // Runway ID                    
  char	  iils[6];                        // ils identifier
  char    name[40];                       // ILS NAME
  int     type;                           // Component type
  SPosition pos;                          // positon lat long alti
  float   freq;                           // Frequency
  float   gsan;                           // Glide slope angle
  float   mDev;                           // Magnetic deviation
  float   rang;                           // range (nm)
  //------------ILS parameters -------------------------------------
  CRunway   *rwy;                         // Associated runway
  CAirport  *apt;                         // Associated airpot
  float   rwyDir;                         // Runway direction
  float   ilsVEC;                         // ILS vector
  float	  radial;                         // Aircraf position
	float	  nmiles;                         // Distance to ILS
  //------------ILS data -------------------------------------------
  ILS_DATA *ilsD;                         // Pointer to runway ILS data
  //------------Markers beacons ------------------------------------
  B_MARK  outM;                           // Outter marker
  B_MARK  medM;                           // Medium marker
  B_MARK  inrM;                           // Inner marker
  //------------ Public methods ------------------------------------
protected:
  void    SetAttributes();                 // Additional attibutes
public:
  CILS(OTYPE qo,QTYPE qa);
  void		Trace(char *op,U_INT FrNo,U_INT key);
	void	  Refresh(U_INT FrNo);							// Update ILS
	CILS*	  IsThisILSOK(float freq);          // Check for tuning
	bool		IsSelected(float freq);
  void    WriteCVS(U_INT gx,U_INT gz,char *sep,SStream &st);
  void    SetGlidePRM();
  //-----Parameters --------------------------------------------------
  void    SetIlsParameters(CRunway *run,ILS_DATA *dt,float dir);
  //-----BEACON MANAGEMENT -------------------------------------------
  char    InMARK(SVector &p, B_MARK &b,CBeaconMark &r);
  inline  char InOUTM(SVector &p,CBeaconMark &r)  {return InMARK(p,outM,r);}
  inline  char InMIDL(SVector &p,CBeaconMark &r)  {return InMARK(p,medM,r);}
  inline  char InINNR(SVector &p,CBeaconMark &r)  {return InMARK(p,inrM,r);}
  //------------------------------------------------------------------
	inline  short GetRefDirection()			{return short(rwyDir);}
	inline  float GetRwyDirection()			{return rwyDir;}
  inline  char* GetName()             {return name;}
  inline  char *GetRWID()             {return irwy;}
  inline  float GetRadial()           {return radial; }
  inline  float GetIlsVector()        {return ilsVEC;}
  inline  float GetNmiles(void)       {return nmiles; }
  inline  float GetMagDev(void)       {return mDev; }
  inline  float GetFrequency(void)    {return freq; }
  inline  float GetRange()            {return rang;}
  inline  void  LinkAPT(CAirport *ap) {apt = ap;}
  inline  bool  IsInRange(void)       {return (rang >= nmiles);	}
  //-----------------------------------------------------------------
  inline  void  SetRecNo(U_INT No)  { recNo = No;}
  inline  U_INT GetRecNo()          { return recNo;}
  //-----------------------------------------------------------------
  inline  bool  SameEND(char *end)      {return (strcmp(end,irwy)  == 0);}
  inline  bool  SameAPT(CAirport *a)    {return (apt == a);}
  inline  SPosition *GetLandingPoint()  {return &ilsD->lndP;}
  inline  SPosition *ObjPosition()      {return &pos;}
  inline  SPosition *GetFarPoint()      {return &ilsD->farP;}
  inline  float GetFeetDistance()       {return ilsD->disF;}
  inline  float GetGlide(void)          {return ilsD->errG;}
	inline  float GetVrtDeviation()				{return ilsD->errG;}
	//-----------------------------------------------------------------
	inline  double	Sensibility()					{return 20;}
	inline	U_CHAR	SignalType()					{return SIGNAL_ILS;}
};
//=======================================================================
//  Structure to draw runway ident
//=======================================================================
struct RwyID {
		SPosition pos;										// Position
    U_CHAR  Paved;                    // Paved runway
    U_CHAR  LetID;                    // letter ID
    U_CHAR  nbTB;                     // Number of THreshold bands
    SVector lPos;                     // left number position
    SVector rPos;                     // right number position
    SVector cPos;                     // Letter position
    SVector lTRH;                     // Left threshold position
    SVector rTRH;                     // Right threshold position
    double  aRot;                     // Rotation angle
		RwyID  *opos;											// Opposit end
};
//=======================================================================
//  RUNWAY LIGHT REQUEST
//=======================================================================
#define RWY_LS_EDGES    (0x00000001)  // Edge lights
#define RWY_LS_CENTER   (0x00000002)  // Center lights
#define RWY_LS_TOUCH    (0x00000004)  // Touch down ligths
#define RWY_LS_BARS     (0x00000008)  // All bars
#define RWY_LS_APPRO    (0x00000010)  // Approach lights 
#define RWY_LS_ENDLT    (0x00000020)  // Everything alighted
//=======================================================================
//  RUNWAY 
//=======================================================================
class CRunway : public CmHead {
  	//-------------RUNWAY attributes --from Disk database -------------
	friend class CDbCacheMgr;
	friend class CDatabaseRWY;
  friend class SqlMGR;
  friend class CILS;
  //-------------------------------------------------------------------
#define ILS_HI_END (0x01)
#define ILS_LO_END (0x02)
  //-------------------------------------------------------------------
protected:
  U_INT recNo;                            // Record number
  char  rapt[10];                         // Airport key
  int   rlen;                             // Runway lenght (feet)
  int   rwid;                             // Runway wide (ft)
  int   rsfc;                             // Surface type
  int   rcon;                             // Surface conditions
  int   rpav;                             // Runway pavement
  int   rpcn;                             // Pavement classification number
  int   rsub;                             // Pavement subgrade
  int   rpsi;                             // Pavement tire PSI
  char  rpat;                             // Traffic pattern direction
  char  rcls;                             // Closed or unused
  char  rpcl;                             // Pilot controlled light
  char  rhid[4];                          // Runway end ID (High)
  SPosition pshi;                         // Lat,long,elevation (high)
  float rhhd;                             // True heading (high)
  float rhmh;                             // Magnetic deviation (high)
  int   rhdt;                             // Displacement threshold (ft)(high)
  int   rhte;                             // Threshold elevation (ft)(high)
  char  rhel;                             // End lights (high)
  char  rhal;                             // Alignment lights (high)
  char  rhcl;                             // Center line lights (high)
  char  rhtl;                             // Touch down zone lights (high)
  char  rhth;                             // Threshold lights (high)
  char  rhli;                             // Edge lights (high)
  char  rhsl;                             // Sequence flashing lights (high)
  char  rhmk;                             // Markings (high)
  char  rhvr;                             // RVR (high)
  int   rhvv;                             // RVV (high)
  char  rhgt;                             // GlideSlope type (high)
  char  rhgc;                             // GlideSlope configuration (high)
  char  rhgl;                             // GlideSlope location (high)
  U_CHAR rh8l[8];                          // 8 light systems
  //---------------------------------------------------------------
  char  rlid[4];                          //End ID (low)
  SPosition pslo;                         // Lat, long, elevation
  float rlhd;                             // True heading (low)
  float rlmh;                             // Magnetic deviation (low)
  int   rldt;                             // Displacement threshold (ft)(low)
  int   rlte;                             // Threshold elevation (ft)(low)
  char  rlel;                             // End lights (low)
  char  rlal;                             // Alignment lights (low)
  char  rlcl;                             // Center line lights (low)
  char  rltl;                             // Touch down zone lights (low)
  char  rlth;                             // Threshold lights (low)
  char  rlli;                             // Edge lights (low)
  char  rlsl;                             // Sequence flashing lights (low)
  char  rlmk;                             // Markings (low)
  char  rlvr;                             // RVR (low)
  int   rlvv;                             // RVV (low)
  char  rlgt;                             // GlideSlope type (low)
  char  rlgc;                             // GlideSlope configuration (low)
  char  rlgl;                             // GlideSlope location (low)
  U_CHAR rl8l[8];                          // 8 light systems
  //----------------------------------------------------------------
  int   rtxc;                             // Texture count
  //------------ Added attributes ----------------------------------
  CRLP       *lpf;                        // Runway light profile
  U_INT       slite;                      // Light switches
  U_INT       mlite;                      // Light marker                   
  U_CHAR      lgCode;                     // Runway code (TP312E)
  U_CHAR      wiCode;                     // Runway letter (TPE312E)
  U_CHAR      Grnd;                       // Ground Index
  U_CHAR      Blend;                      // Need blending
  U_CHAR      Paved;                      // Paved runway
  SVector     scl;                        // Scale letter
  RwyID       pID[2];                     // Ident parameters both end
  //----------------------------------------------------------------
  ILS_DATA    ilsD[2];                    // Ils data for both ends
  U_CHAR      ilsT;                       // ils type (HI or LO)
  //-------------Private methods -----------------------------------
  U_CHAR  GetGroundIndex();
  U_CHAR  GetIdentIndex(char kl);
  //------------ Public methods ------------------------------------
protected:
  void    SetAttributes();
public:
  CRunway(OTYPE qo,QTYPE qa);
 ~CRunway();
 //------------------------------------------------------------------
 CRunway *GetNext()     {return (CRunway*)CmHead::Cnext;}
 //------------------------------------------------------------------
  void    InitILS(CILS *ils);
  void		Trace(char *op,U_INT FrNo,U_INT key);
  int     GetCode();
  int     GetLetter();
  void    WriteCVS(U_INT No,U_INT gx,U_INT gz,char *sep,SStream &st);
  //------Light switching ------------------------------------------
  char    ChangeLights(char ls);
  //----------------------------------------------------------------
  inline  U_CHAR    *GetHiLightSys()    {return rh8l;}
  inline  U_CHAR    *GetLoLightSys()    {return rl8l;}
  inline  SPosition *GetLandPos(char p) {return &ilsD[p].lndP;}
  inline  char       GetIlsIndex()      {return ilsT;}
  inline  ILS_DATA  *GetIlsData(char p) {return (ilsD + p);}
  //----------------------------------------------------------------
  inline  char      *GetHiEnd()  {return rhid; }
  inline  char      *GetLoEnd()  {return rlid; }
  inline float       GetHiDir()  {return rhhd; }
  inline SPosition   GetHiPos()  {return pshi; }
  inline float       GetLoDir()  {return rlhd; }
  inline SPosition   GetLoPos()  {return pslo; }
  inline int         GetLenghi() {return rlen; }
  inline int         GetWidthi() {return rwid;}
  inline int         GetHiDSP()  {return rhdt; }
  inline int         GetLoDSP()  {return rldt; }
  inline int         GetSurface(){return rsfc; }
  inline U_CHAR      GroundIndex() {return Grnd; }
  inline U_CHAR      GetWiCode() {return wiCode;}
  inline U_CHAR      GetLgCode() {return lgCode;}
  //---------Runway category       --------------------------------
  inline bool        NoCenter()     {return (rhcl  == 0);}
  inline bool        HasCenter()    {return (rhcl  != 0);}
  inline bool        NoEdge()       {return (rhli  == 0);}
  inline bool        NoPrecision()  {return (rhmk  <  7);}
  inline bool        LoLightEnd()   {return (rlel  != 0);}
  inline bool        HiLightEnd()   {return (rhel  != 0);}
  inline bool        HasHiRTH()     {return (rhth  != 0);}
  inline bool        HasLoRTH()     {return (rlth  != 0);}
  inline bool        HasHiETH()     {return (rhel  != 0);}
  inline bool        HasLoETH()     {return (rlel  != 0);}
  inline bool        HasHiTDZ()     {return (rhtl  != 0);}
  inline bool        HasLoTDZ()     {return (rltl  != 0);}
  //-----------------------------------------------------------------
  inline double      GetWidth()   {return double(rwid);}
  inline double      GetLength()  {return double(rlen);}
  //-----------------------------------------------------------------
  inline  void       SetRecNo(U_INT No)   {recNo = No;}
  inline  U_INT      GetRecNo()           {return recNo;}
  //------------Light system management -----------------------------
  inline  void       SetRLP(CRLP *p)      {lpf = p;}
  inline  void       SetROT(double a, int p) {pID[p].aRot = a;}
  inline  void       SetLPS(SVector v,int p) {pID[p].lPos = v;}
  inline  void       SetRPS(SVector v,int p) {pID[p].rPos = v;}
  inline  void       SetCPS(SVector v,int p) {pID[p].cPos = v;}
  inline  void       SetLTH(SVector v,int p) {pID[p].lTRH = v;}
  inline  void       SetRTH(SVector v,int p) {pID[p].rTRH = v;}
  inline  void       SetNBT(U_CHAR  n,int p) {pID[p].nbTB = n;} 
  inline  void       SetPaved(U_CHAR p)      {Paved = p;}
  //------------Light system management -----------------------------
	inline  RwyID			*GetEndDEF(char n)				{return pID + n;}
  inline  CRLP      *GetRLP()                 {return lpf;}
  inline  U_CHAR     GetNBT(int p)            {return pID[p].nbTB;}
  inline  U_CHAR     GetLET(int p)            {return pID[p].LetID;}
  inline  double     GetROT(int p)            {return pID[p].aRot;}
  inline  void       GetLPS(SVector &v,int p) {v = pID[p].lPos;}
  inline  void       GetRPS(SVector &v,int p) {v = pID[p].rPos;}
  inline  void       GetCPS(SVector &v,int p) {v = pID[p].cPos;}
  inline  void       GetLTR(SVector &v,int p) {v = pID[p].lTRH;}
  inline  void       GetRTR(SVector &v,int p) {v = pID[p].rTRH;}
  inline  U_CHAR     GetPaved()               {return Paved;}
  //-----------Light request ----------------------------------------
  inline  void       LiteON(U_INT m)      {slite |= m;}
  inline  void       LiteOF(U_INT m)      {slite &= (-1 - m);}
  inline  U_INT      Alighted(U_INT m)      {return (slite & m);}
  //-----------------------------------------------------------------
  inline  void       SetXscale(double x)  {scl.x = x;}
  inline  void       SetYscale(double y)  {scl.y = y;}
  inline  double     GetXscale()          {return scl.x;}
  inline  double     GetYscale()          {return scl.y;}
  //-----------------------------------------------------------------
  inline void        SetBlend(char b)     {Blend = b;}
  inline bool        NeedBlend()          {return (Blend == 1);}
  inline bool        HasProfile()         {return (lpf != 0);}
	//---Setting parameters -------------------------------------------
  void  SetNumberBand(int bw);
	void	SetLETTER(int k,U_CHAR rs);
	void	SetNUMBER(int k,U_CHAR rs,double nmx,double nmy);
  //---Drawing ------------------------------------------------------
	void	DrawLetter(char No);
	void	DrawNumber(char cr);
	void	DrawBand(int nbp);
	void	DrawThreshold(SVector &sl,U_CHAR rs);
	void	DrawRID(CAptObject *apo,int rs,char *rid);
	void	DrawDesignators(CAptObject *apo);
};
//=======================================================================
//  AIRPORT 
//  NOTE: To save memory not all fields are decoded.
//        If needed just uncomment them and modify the DecodeRecord 
//        in consequence
//=======================================================================
  //----airport properties ----------------------------------------------
#define TC_LONG_RWY  0x01                 // Long runways
#define TC_HARD_RWY  0x02                 // Hard runways
#define TC_EDIT_BOX  0x10                 // Under edition
#define TC_PCHANGES  0x20                 // Changed
//=======================================================================
class CAirport : public CmHead {
  	//-------------RUNWAY attributes --from Disk database -------------
	friend class CDbCacheMgr;
	friend class CDatabaseAPT;
  friend class SqlMGR;
  friend class CFuiVectorMap;
  //--------------Airport attributes from disk --------------------
protected:
  U_INT recNo;                            // Record number
  //---------------------------------------------------------------
  char  akey[10];                         // Airport key
  char  afaa[4];                          // FAA ID
  char  aica[5];                          // ICAO ID
  char  name[40];                         // Name
  char  actr[3];                          // Country
  char  asta[3];                          // State
// char  acty[20];                         // County                      NOT USED
  short atyp;                             // type
  SPosition apos;                         // Lat-long-elevation
  char  aown;                             // ownership
  char  ause;                             // Usage                      NOT USED    
//  char  argn[5];                          // Region                     NOT USED
//  char  antm[20];                         // Notam facility             NOT USED
//  char  afss[20];                         // FSS facility
  float amag;                             // Magnetic variation
  int   aalt;                             // Traffic altitude (ft MSL)
  char  afsa;                             // Flight service   available
  char  antd;                             // Notam D available
  char  aata;                             // Air traffic control available
  char  aseg;                             // Segment circle available
  char  alnd;                             // Landing fee charged
  char  amjc;                             // Military/civil jooint use
  char  amln;                             // Military landing right
  char  acus;                             // Custom entry airport
  short afue;                             // Fuel type available
  char  afrm;                             // Frame service available  
  char  aeng;                             // Engine service available
  char  abto;                             // Bottled oxygen available
  char  ablo;                             // Bulk oxygen available
  char  alen;                             // Beacon lens color
  short acft;                             // Based aircraft types
  int   aaco;                             // Annual commercial operations
  int   aaga;                             // Annual general aviation operations
  int   aamo;                             // Annual military operations
  int   aatf;                             // Airport attendance
  int   aats;                             // Airport attendance start time
  int   aate;                             // Airport attendance close time
  char  altf;                             // Airport lighting
  int   alts;                             // Airport lighting start time
  int   alte;                             // Airport lighting end time
  char  atyc;                             // Airport type
  char  aicn;                             // Airport icon
  char  anrw;                             // Number of runways
  int   alrw;                             // longest runway (ft)
  char  avor;                             // VOR colocated on airport
  char  aatc;                             // ATC service available
  char  ails;                             // ILS approach available
  //----------Other variables ----------------------------------------------------
  U_CHAR  MapTp;                          // Map type
  U_CHAR  Prop;                           // Runway Properties
  U_CHAR  NbILS;                          // Number of ILS
  U_SHORT cTyp;                           // Radio com type
  //----------Various paramters --------------------------------------------------
  CCOM  *atis;                            // Athis radio
  CCOM  *rdep;                            // Departure
  Tag   Metar;                            // Metar key
  float cfrq;                             // Clearance frequency
  float radial;                           // Radial to airport
  float nmiles;                           // Distance in mile
  float pDis;                             // Squared distance
  //----------Queue of Runways ---------------------------------------------------
  CAptObject *apo;                        // Airport object
  ClQueue     rwyQ;                       // Airport runway queue
  //-------------------Airport methods --------------------------------------------
protected:
  void  SetAttributes();                  // Additional attributes
public:
  CAirport(OTYPE qo,QTYPE qa);
 ~CAirport();
  void		    Trace(char *op,U_INT FrNo,U_INT key);
  int         SetMapType(void);
  void        Refresh(U_INT FrNo);
  short       GetFuelGrad();
  char       *GetAptName();
  void        RebuildLights(CRunway *rwy);
  void        SaveProfile();
  void        WriteCVS(U_INT No,U_INT gx,U_INT gz,char *sep,SStream &st);
  //---------inline ----------------------------------------
  inline float      GetLatitude(void) {return apos.lat;}
  inline float      GetLongitude(void){return apos.lon;}
  inline float      GetRadial()       {return radial;}
  inline float      GetNmiles()       {return nmiles;}
  inline  int       GetMapNo(void)    {return MapTp;}
  inline SPosition  GetPosition(void) {return apos; }
  inline SPosition *ObjPosition()     {return &apos;}
  inline  char*     GetFaaID(void)    {return afaa; }
  inline  char*     GetFaica(void)    {return aica;}
  inline  char*     GetIdent(void)    {return(*afaa)?(afaa):(aica);}
  inline  char*     GetName(void)     {return name; }
  inline  char*     GetKey(void)      {return akey; }
  inline  char*     GetCountry()      {return actr;}
  inline  char*     GetEtat()         {return asta;}
  inline  void      RazInEDIT(void)   {Prop &=(-1 - TC_EDIT_BOX);}
  inline  float     GetMagDev(void)   {return amag; }
  inline  float     GetElevation()    {return apos.alt;}
  inline  char      GetOwner()        {return aown;}
  inline  float     GetClearanceFreq(){return cfrq;}
  inline  char      GetILSfacility()  {return NbILS;}
  inline  void      AddOneILS()       {NbILS++;}
  inline  int       GetLighting()     {return altf;}
  inline  int       GetIcon()         {return aicn;}
  inline  bool      IsSelected()      {return (0 != apo);}
  inline  bool      NotSelected()     {return (0 == apo);}
  inline  int       UnderEdit()       {return (Prop & TC_EDIT_BOX);}
  //----RADIO INTERFACE ---------------------------------------------
  inline void       SetATIS(CCOM *r)  {atis = r;}
  inline void       SetRDEP(CCOM *r)  {rdep = r; cfrq = r->GetFrequency();}
  inline CCOM      *GetATIS()         {return atis;}
	inline CAptObject *GetAPO()         {return apo;}
  //-----------------------------------------------------------------
	inline void       SetElevation(float e)	{   apos.alt = e;}
  inline void       SetAPO(CAptObject *p) {apo = p;}
  inline  U_CHAR    GetBeacon()       {return (alen & 0x07);}
  //-----------------------------------------------------------------
	inline  void			SetNmiles(float m){nmiles = m;}
	inline  void      SetLongRWY(void)  {Prop |= TC_LONG_RWY;}
  inline  void      SetHardRWY(void)  {Prop |= TC_HARD_RWY;}
  inline  void      SetInEDIT(void)   {Prop |= TC_EDIT_BOX;}
  //-----------------------------------------------------------------
  inline  void      SetMetar(Tag m)   {Metar = m;}
  inline  Tag       GetMetar()        {return Metar;}
  //-----------------------------------------------------------------
  inline  void      SetRecNo(U_INT No)  { recNo = No;}
  inline  U_INT     GetRecNo()          { return recNo;}
  //---------------Runways methods ----------------------------------
  void        AddRunway(CRunway *rwy);          // Add one runway
  CRunway    *FindRunway(char *idn);            // Find runway by end ident
  //-----------------------------------------------------------------
  inline double     GetAltitude()      {return apos.alt;}
  inline char *     GetIdentity()      {return (*afaa)?(afaa):(aica);}
  //-----------------------------------------------------------------
	ClQueue    *GetRWYQ()								 {return &rwyQ;}
  CRunway    *GetNextRunway(CRunway *rwy);      // Get next runway
  //-----------------------------------------------------------------
  bool        HasRunway()              {return rwyQ.NotEmpty();}
  //------------------------------------------------------------------
};
//================================================================================
//  CObstruction
//================================================================================
class CObstacle: public CmHead {
  //-------------RUNWAY attributes --from Disk database -------------
	friend class CDbCacheMgr;
	friend class CDatabaseOBS;
protected:
  //--ATTRIBUTES form database --------------------------------------
  char okey[10];                      // Obstacle unique key
  char osta[4];                       // State (3char only)
  char octy[16];                      // City
  char onam[21];                      // Name
  SPosition pos;                      // Coordinates
  int   otyp;                         // Type
  char  ostb;                         // Strobe indicator
  float ofrq;                         // Frequency
  //-----------------------------------------------------------------
  float pDis;                         // Distance to Plane (squared)
  //-----------------------------------------------------------------
public:
  CObstacle(OTYPE qo,QTYPE qa);
  void  Trace(char *op,U_INT FrNo,U_INT key);
  //----------------------------------------------------------------
  inline float      GetPDIS()           {return pDis;}
  inline void       SetPDIS(float d)    {pDis = d;}
  inline SPosition *ObjPosition()       {return &pos;}
};
//================================================================================
//  CCountry for country names
//================================================================================
class CCountry {
  friend class CDbCacheMgr;
  friend class SqlMGR;
  //-------------Attributes ---------------------------------------
  char    cuid[4];                // country key
  char    cnam[40];               // Country name
  //---------------------------------------------------------------
public:
  void  WriteCVS(char *sep,SStream &st);
  //--------------inline ------------------------------------------
  inline  char*   GetKey()  {return cuid;}
  inline  char*   GetName() {return cnam;}
  inline  void    SetLimit(){cnam[39] = 0;}
};
//================================================================================
//  CState for states
//================================================================================
class CState {
  friend class CDbCacheMgr;
  friend class SqlMGR;
  //------------Attributes -----------------------------------------
  char   skey[5];                 // State key
  char   sctr[3];                 // country key
  char   nsta[4];                 // State key
  char   name[40];                // state name
public:
  void   WriteCVS(char *sep,SStream &st);
  //------------inline ---------------------------------------------
  inline  char*   GetSKey() {return skey;}
  inline  char*   GetCKey() {return sctr;}
  inline  char*   GetName() {return name;}
  inline  char*   GetState(){return nsta;}
  inline  void    SetLimit(){name[39] = 0;}
};
//================================================================================
//
// Database manager, a singleton class that provides centralized
//   access to all available databases without the need for 
//   global variables
//===============================================================================
class CDatabaseManager {
private:
  CDatabaseManager (void);
  static CDatabaseManager instance;

public:
  void   Init (void);
  void   Cleanup (void);
  static CDatabaseManager& Instance() { return instance; }

  CDatabase* GetDatabase (Tag id);
  CDatabase* GetAPTDatabase (void);
  CDatabase* GetAtsRouteDatabase (void);
  CDatabase* GetCOMDatabase (void);
  CDatabase* GetCTYDatabase (void);
  CDatabase* GetSTADatabase (void);
  CDatabase* GetILSDatabase (void);
  CDatabase* GetNAVDatabase (void);
  CDatabase* GetOBSDatabase (void);
  CDatabase* GetRWYDatabase (void);
  CDatabase* GetWPTDatabase (void);

protected:
  enum {
    DB_AIRPORT      = 'arpt',
    DB_ATS_ROUTE    = 'atsr',
    DB_COMM         = 'comm',
    DB_COUNTRY      = 'ctry',
    DB_STATE        = 'stat',
    DB_ILS          = 'ils_',
    DB_NAVAID       = 'navd',
    DB_OBSTRUCTION  = 'obst',
    DB_RUNWAY       = 'runw',
    DB_WAYPOINT     = 'wayp'
  };
  std::map<Tag, CDatabase*>  db;
};
#endif // DATABASE_H_
//============================END OF FILE =================================================