/*
 * Fui.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
 * Copyright 2007      Jean Sabatier
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
#ifndef FUIPART_H
#define FUIPART_H
//==================================================================================
#include "../Include/Fui.h"
#include "../Include/Database.h"
#include "../Include/Robot.h"
//==================================================================================
class CKeyDefinition;
class SJoyDEF;
class CSimButton;
class CSimAxe;
class CAirport;
class CWaypoint;
class C2valSlot;
class CWPoint;
//=============================================================================
extern char *ObjBTN[];
extern char *AptBTN[];
extern char  btnLOK[];
//=============================================================================
//  CText is used by CEditor to support the callback functions:
//  GetLine(char l)       Must return the line number l
//  SetPosition(short p)  Set cursor position inside current line of text
//  Object using CEditor may inherit from the CText to supply the callback
//=============================================================================
class CText {
  //--- METHODS --------------------------------------------
public:
  virtual   void  CursorAt(short l,short k)  {}
  virtual   char *NextLine(char l)       {return 0;}
  virtual   char *PrevLine(char l)       {return 0;}
};
//=============================================================================
//  Class CEditor to edit a set of text lines
//  NOTE:  All lines must be of the same maximum size
//=============================================================================
class CEditor {
  //---ATTRIBUTES ------------------------------------------
  CText *obj;                             // Text object
  char  *txt;                             // Text to edit
  char   lin;                             // Line number
  short  lim;                             // Limit position
  short  pos;                             // Cursor position
  //---METHODS ---------------------------------------------
public:
  CEditor(short s,CText *t);              // constructor
  //--------------------------------------------------------
  void      SetLine(char *t,char l);      // Init text line
  int       InsertCharAsFirst(char kar, char *tex, char lnb, short p);
  int       InsertCharInLine (char kar, char *tex, char lnb, short p);
  int       PushLastCharInNextLine(char *tex,char lnb);     
  //--------------------------------------------------------
  int       EndMark(char *tex);
  void      KeyNXT();
  void      KeyPRV();
  void      Key(U_INT k,short p);                
};
//=============================================================================
//  Class CCtyLine to display countries in directory box
//=============================================================================
class CCtyLine: public CSlot {
  //-------------Attributes ------------------------------
  char            cuid[4];                // country key
  //--------------inline ---------------------------------
public:
  CCtyLine(): CSlot(1)  {}
  //-------------------------------------------------------
  inline  void    SetSlotKey(char *key)  {strncpy(cuid,key,4);}
  inline  char*   GetSlotKey()					 {return cuid;}
};
//=============================================================================
//  Class CStaLine to display States in directory box
//=============================================================================
class CStaLine: public CSlot {
  //-----------Attributes --------------------------------
  char            skey[6];                // State key
  char            ckey[4];                // Country key
  //-----------inline ------------------------------------
public:
  CStaLine(): CSlot(1) {}
  //-------------------------------------------------------
  inline  void    SetSlotKey(char *sk)   {strncpy(skey,sk,6);}
  inline  char   *GetSlotKey()					 {return skey;}
  //-------------------------------------------------------
  inline  void    SetCTY(char *ck)   {strncpy(ckey,ck,4);}
  //------------------------------------------------------
  inline  char   *GetCTY()           {return ckey;}
};
//=============================================================================
//  Class CAptLine to display Airports in directory box
//=============================================================================
class CAptLine: public CSlot {
  //----------Attributes ---------------------------------
  SPosition       Pos;                // Position
  char            akey[10];           // Airport key
  char            aica[5];            // ICAO identification
  char            ifaa[4];            // FAA
  char            ncty[3];            // Country ident
  char            nsta[3];            // State ident
  char           *otxt;               // Owner text
  char           *clab;               // Country label
  U_CHAR          owns;               // Ownership
  //-------------------------------------------------------
public:
  //-------------------------------------------------------
  CAptLine(): CSlot(1) {akey[0] = 0;}
  void         Print(CFuiList *w,U_CHAR ln);
  //----------inline --------------------------------------
  void            SetAkey(char *k)    {strncpy(akey,k, 10);}
  void            SetAica(char *ic)   {strncpy(aica,ic, 5);}
  void            SetIfaa(char *id)   {strncpy(ifaa,id, 4);}
  void            SetActy(char *ct)   {strncpy(ncty,ct, 3);}
  void            SetAsta(char *st)   {strncpy(nsta,st, 3);}
  void            SetOwtx(char *ot)   {otxt = ot;}
  void            SetAown(U_CHAR ow)  {owns = ow;}
  void            SetClab(char *lb)   {clab = lb;}
  void            SetPosition(SPosition p) {Pos = p;}
  //--------------------------------------------------------
  char           *GetSlotKey()        {return akey;}
  char           *GetAica()           {return aica;}
  char           *GetIfaa()           {return ifaa;}
  char           *GetActy()           {return ncty;}
  char           *GetAsta()           {return nsta;}
  char           *GetOtxt()           {return otxt;}
  char           *GetClab()           {return clab;}
  U_CHAR          GetAown()           {return owns;}
  void            GetPosition(SPosition &p)     {p = Pos;}
};
//=============================================================================
//  Class CNavLine to display NAV-NDB in directory box
//=============================================================================
class CNavLine: public CSlot {
  //-----------Attributes ---------------------------------------
  SPosition       Pos;                // Position
  U_SHORT         type;               // Type of navaid
  char            nkey[10];           // NAV key
  char            naid[ 5];           // ICAO identification
  char            ncty[ 3];           // Country ident
  char            nsta[ 3];           // State ident
  char           *ntyp;               // Navaid type
  char           *clab;               // Country label
public:
  //-------------------------------------------------------------
  CNavLine() : CSlot(1) {nkey[0] = 0;}
  void             Print(CFuiList *w,U_CHAR ln);
  //-----------inline -------------------------------------------
  void            SetSlotKey(char *k) {strncpy(nkey,k,10);}
  void            SetType(U_SHORT tp) {type = tp;}
  void            SetVaid(char *id)   {strncpy(naid,id, 5);}
  void            SetVcty(char *ct)   {strncpy(ncty,ct, 3);}
  void            SetVsta(char *st)   {strncpy(nsta,st, 3);}
  void            SetVtyp(char *vt)   {ntyp = vt;}
  void            SetClab(char *lb)   {clab = lb;}
  void            SetPosition(SPosition p) {Pos = p;}
  //-------------------------------------------------------------
  char           *GetSlotKey()        {return nkey;}
  U_SHORT         GetType()           {return type;}
  char           *GetVaid()           {return naid;}
  char           *GetVcty()           {return ncty;}
  char           *GetVsta()           {return nsta;}
  char           *GetClab()           {return clab;}
  char           *GetVtyp()           {return ntyp;}
  void            GetPosition(SPosition &p)     {p = Pos;}
  //-------------------------------------------------------------
  bool            IsNDB() {return ((type & NAVAID_TYPE_NDB) != 0);}
  bool            IsVOR() {return ((type & NAVAID_TYPE_NDB) == 0);}
};
//=============================================================================
//  Class CWptLine to display WPT in directory box
//=============================================================================
class CWptLine: public CSlot {
  //-----------Attributes ---------------------------------------
  SPosition       Pos;                // Position
  U_SHORT         wtyp;               // Type of waypoint
  char            wkey[10];           // WPT key
  char            waid[ 5];           // ICAO identification
  char            wcty[ 3];           // Country ident
  char            wsta[ 3];           // State ident
  char           *wlab;               // Country label
public:
  //-------------------------------------------------------------
  CWptLine() : CSlot(1) {wkey[0] = 0;}
  void            Print(CFuiList *w,U_CHAR ln);
  //-----------inline -------------------------------------------
  void            SetSlotKey(char *k) {strncpy(wkey,k,10);}
  void            SetWtyp(U_SHORT tp) {wtyp = tp;}
  void            SetWaid(char *id)   {strncpy(waid,id, 5);}
  void            SetWcty(char *ct)   {strncpy(wcty,ct, 3);}
  void            SetWsta(char *st)   {strncpy(wsta,st, 3);}
  void            SetClab(char *lb)   {wlab = lb;}
  void            SetPosition(SPosition p) {Pos = p;}
  //-------------------------------------------------------------
  char           *GetSlotKey()        {return wkey;}
  U_SHORT         GetWtyp()           {return wtyp;}
  char           *GetWaid()           {return waid;}
  char           *GetWcty()           {return wcty;}
  char           *GetWsta()           {return wsta;}
  char           *GetClab()           {return wlab;}
  //-------------------------------------------------------------
  void            GetPosition(SPosition &p)     {p = Pos;}
  inline   float  GetLatitude()       {return Pos.lat;}
  inline   float  GetLongitude()      {return Pos.lon;}

};
//=============================================================================
//  Class CFlpLine to display Flight Plan in directory box
//=============================================================================
class CFlpLine: public CSlot {
  //-----------Attributes -----------------------------------
  CWPoint      *wPnt;                 // Current waypoint
  //---------------------------------------------------------
  char            Iden[6];            // Identity
  char            Mark[2];            // Crossed marker
  char            Dist[10];           // Distance
	char						Dirt[6];						// Direction to
  char            Alti[16];           // Altitude
  char            Elap[16];           // Elapse time
  char            Etar[16];           // Arrival time
public:
  //---------------------------------------------------------
	CFlpLine(): CSlot(1) {wPnt = 0;}
  void            Print(CFuiList *w,U_CHAR ln);
  void            Edit();
  //------------inline --------------------------------------
  void            SetIden(char *id)   {strncpy(Iden,id, 5);}
  void            SetMark(char *mk)   {strncpy(Mark,mk, 2);}
  void            SetDist(char *di)   {strncpy(Dist,di,10);}
	void						SetDirt(char *dr)   {strncpy(Dirt,dr,6);}
  void            SetAlti(char *al)   {strncpy(Alti,al,12);}
  void            SetWPT(CWPoint *w){wPnt = w;}							// OBSOLETE
  void            SetElap(char *el)   {strncpy(Elap,el,12);}
  void            SetEtar(char *ar)   {strncpy(Etar,ar,14);}
  //---------------------------------------------------------
  char           *GetIden()           {return Iden;}
  char           *GetMark()           {return Mark;}
  char           *GetDist()           {return Dist;}
	char					 *GetDirt()						{return Dirt;}
  char           *GetAlti()           {return Alti;}
  char           *GetElap()           {return Elap;}
  char           *GetEtar()           {return Etar;}
  CWPoint        *GetWPT()            {return wPnt;}						// OBSOLETE
};
//=============================================================================
//  Class CFpnLine to display Flight Plan name
//=============================================================================
class CFpnLine: public CSlot {
  //-----------Attributes ----------------------------------
  char fname[64];
public:
  CFpnLine() : CSlot(1) {}
  void            Print(CFuiList *w,U_CHAR ln);
  bool            Match(void *k);  
  //------------inline -------------------------------------
  void          SetFile(char *f)  {strncpy(fname,f,32); fname[31] = 0;}
  //--------------------------------------------------------
  char         *GetFile()         {return fname;}
};
//=============================================================================
//  Class CComLine to display Com in Airport detail
//=============================================================================
class CComLine: public CSlot {
  //---------Attributes --------------------------------------
  U_INT         mask;               // Type mask
  U_CHAR        signal;             // COM subtype
  char          data[16];           // Type of com
  char          dfrq[8];            // Frequency
  float         freq;               // Frequency
  RD_COM        comInx;             // Com Index
  char          irwy[4];            // Ruway end identification for ILS
public:
  //----------------------------------------------------------
  CComLine() : CSlot(1) {}
  void            Print(CFuiList *w,U_CHAR ln);
  //---------inline ------------------------------------------
  inline  void    SetFreq(float fq)       {freq = fq;}
  inline  void    SetMask(U_INT msk)      {mask = msk;}
  inline  void    SetIden(char *id)       {strncpy(irwy,id,4);}
  inline  U_CHAR  Signal()                {return signal;}
  inline  float   GetFreq()               {return freq;}
  inline  void    SetIRWY(char *r)        {strncpy(irwy,r,4);}
  inline  U_INT   GetMask()               {return mask;}
  inline  void    SetIndex(RD_COM x)      {comInx = x;}
  //----------------------------------------------------------
  char        *GetData()							{return data;}
  char        *GetDfrq()							{return dfrq;}
	char        *GetIdent()							{return irwy;}
  //----------------------------------------------------------
  void  FillCom();
  void  FillILS();
  Tag   IsaILS();
  Tag   IsaCOM();
};
//=============================================================================
//  Class CRwyLine to display Runway in Airport detail
//=============================================================================
class CRwyLine: public CSlot {
  friend class CDatabaseRWY;
  friend class CDbCacheMgr;
  //---------Attributes ----------------------------------------
  char      rapt[10];                     // Airport key
  U_INT     rwlg;                         // runway lenth
  U_INT     rwid;                         // Runway wid
  RWEND     Hend;                         // Hight end description
  RWEND     Lend;                         // Lower end description
  char      data[16];                     // Edited data
  char      leng[10];                     // Length
  //--------inline ----------------------------------------------
public:
  //-------------------------------------------------------------
  CRwyLine() : CSlot(1) {}
  void      Print(CFuiList *w,U_CHAR ln);
  //-------------------------------------------------------------
  void      SetSlotKey(char *k)       {strncpy(rapt,k,10);}
  void      SetLeng(U_INT lg)         {rwlg = lg;}
  void      SetHend(RWEND end)        {Hend = end;}
  void      SetLend(RWEND end)        {Lend = end;}
  void      SetRWID(int k)            {rwid = k;}
	void			SetILS(CComLine *c);
  void      EditRWY();
  void      AdjustEnd(float scale,short tx,short ty);
  void      ComputeCorner(short mx,short my);
  void      GetEnd01(int *px,int *py);
  void      GetEnd02(int *px,int *py);
  void      GetEnd03(int *px,int *py);
  void      GetEnd04(int *px,int *py);
	U_CHAR		CheckEnd(char *id,RWEND **end);		///int *px,int *py, char **d);
  //-------------------------------------------------------------
  inline    int GetDXH()    {return Hend.dx;}
  inline    int GetDYH()    {return Hend.dy;}
  inline    int GetDXL()    {return Lend.dx;}
  inline    int GetDYL()    {return Lend.dy;}
  //-------------------------------------------------------------
  inline  char     *GetSlotKey()      {return rapt;}
  inline  int       GetLeng()         {return rwlg;}
  inline  SPosition GetHiPosition()   {return Hend.pos;}
  inline  char     *GetHiEndID()      {return Hend.rwid;}
  inline  char     *GetLoEndID()      {return Lend.rwid;}
  inline  char     *GetData()         {return data;}
  inline  char     *GetElng()         {return leng;}
};
//=============================================================================
//  Class CRwyLine to display Aircraft in Selection list
//=============================================================================
class CAirLine: public CSlot {
  //-------Attributes -------------------------------------------
  char       Nfo[64];                 // File name
  SBitmap   *bmap;                    // Bitmap structure
  //------------method ------------------------------------------
public:
  //-------------------------------------------------------------
  CAirLine() : CSlot(1) {*Nfo = 0;}
  void       Print(CFuiList *w,U_CHAR ln);
  //-------------------------------------------------------------
  inline  void      SetNFO(char *n) {strncpy(Nfo,n,64); Nfo[63];}
  inline  char     *GetNFO()        {return Nfo;}
  //-------------------------------------------------------------
  inline  void      SetBitmap(SBitmap *bmp)   {bmap = bmp;}
  inline  void      Clean()                   {if (bmap) delete bmap;}
  inline  SBitmap  *GetBitmap()               {return bmap;}
};
//=============================================================================
//  Class CGasLine to display Tanks to fill in FUEL LOADOUT
//=============================================================================
class CGasLine: public CSlot {
  //----- Attributes --------------------------------------------
  CFuelCell   *cel;
  //-------------------------------------------------------------
public:
  CGasLine(CFuelCell *c): CSlot(1)  {cel = c;}
  CGasLine(): CSlot(1)              {cel = 0;}
  void       Print(CFuiList *w,U_CHAR ln);
  void       Title(CFuiList *w);
  //-------------------------------------------------------------
  inline void       SetCell(CFuelCell *c) {cel = c;}
  inline CFuelCell *GetCell()             {return cel;}
};
//=============================================================================
//  Class CLodLine to display variable LOADOUT
//=============================================================================
class CLodLine: public CSlot {
  //----- Attributes -------------------------------------------
  CLoadCell   *lod;
  //------------------------------------------------------------
public:
  CLodLine(CLoadCell * c): CSlot(1) {lod = c;}
  CLodLine() : CSlot(1)             {lod = 0;}
  void        Print(CFuiList *w,U_CHAR ln);
  //------------------------------------------------------------
  inline void       SetCell(CLoadCell *c) {lod = c;}
  inline CLoadCell *GetCell()             {return lod;}
};
//=============================================================================
//  Class CMapLine to display Sectional chart list
//=============================================================================
class CMapLine: public CSlot {
  //----- Attributes ------------------------------------------
  //----- Methods ---------------------------------------------
public:
  CMapLine(): CSlot(1) {}
};
//=============================================================================
//  Class CSubLine to display Subsystems in probe windows
//=============================================================================
class CSubLine: public CSlot{
  //-------Attribute --------------------------------------------
  CSubsystem *aSub;                     // Subsystem
  char        tyst[8];                  // Type 
  Tag         iden;                     // ident
public:
  //-------------------------------------------------------------
  CSubLine() : CSlot(1) { aSub = 0; }
  void                  Print(CFuiList *w,U_CHAR ln);
  //-----------method -------------------------------------------
  inline    void        SetSubsystem(CSubsystem *obj) {aSub = obj; }
  inline    CSubsystem *GetSubsystem()  {return  aSub;}
  inline    void        SetIden(Tag id) {iden = id;}
  inline    Tag         GetIden()       {return iden;}
  inline    void        SetType(char *t){strncpy(tyst,t,8);}
};
//=============================================================================
//  Class CPidLine to display PID in tuning windows
//=============================================================================
class CPidLine: public CSlot{
  //-------Attribute --------------------------------------------
  CPIDbox    *aPID;                     // PID
public:
  //-------------------------------------------------------------
  CPidLine() : CSlot(1) {aPID = 0;}
  //-----------method -------------------------------------------
  inline    void        SetPID(CPIDbox *pid)  {aPID = pid;}
  inline    CPIDbox    *GetPID()        {return aPID;}
};
//=============================================================================
//  Class CPipLine to display PID parameters in tuning windows
//=============================================================================
class CPipLine: public CSlot{
  //-------Attribute --------------------------------------------
  CPIDbox     *Pid;
  Tag          PRM;                     // Parmeter identity
public:
  //-------------------------------------------------------------
  CPipLine() : CSlot(1) {Pid = 0;}
  void              Print(CFuiList *w,U_CHAR ln);
  //-------------------------------------------------------------
  inline  void      SetPID(CPIDbox *b)  {Pid = b;}
  inline  void      SetPRM(Tag t)       {PRM = t;}
  inline  Tag       GetPRM()            {return  PRM;}
  inline  CPIDbox  *GetPID()            {return  Pid;}
};

//=============================================================================
//  Class CKeyLine to display Keyboard in Key windows
//=============================================================================
class CKeyLine: public CSlot {
  //--------Attributes ------------------------------------
  CKeyDefinition *kdef;                 // Key definition        
  char    ktext[32];                    // keyboard designation
  char    jtext[32];                    // Joystick assignation
  Tag     iden;                         // Key tag
  int     code;                         // Key code
  //-------------------------------------------------------
public:
  CKeyLine() : CSlot(1) {ktext[0] = 0; jtext[0] = 0;}
  void    Print(CFuiList *w,U_CHAR ln);
  bool    Match(void *k)  {return ((CKeyDefinition *)k == kdef);}
  //---------methods --------------------------------------
  void  ClearKey();
  void  SetJoysDef(CSimButton *sbt);
  void  SetCode(int c);
  void            SetKText(char *txt) {strncpy(ktext,txt,31);}
  char           *GetKText()          {return ktext;}
  void            SetJText(char *txt) {strncpy(jtext,txt,31);}
  char           *GetJText()          {return jtext;}
  void            SetIden(Tag t)      {iden = t;}
  Tag             GetIden()           {return iden;}
  void            SetDef(CKeyDefinition *d) {kdef = d;}
  CKeyDefinition *GetKDef()           {return kdef;}
  int             GetCode()           {return code;}
};
//=============================================================================
//  Class CAxeLine to display Joystick axe in Axe windows
//=============================================================================
class CAxeLine: public CSlot {
  //-------ATTRIBUTES-------------------------------------
  CSimAxe *axe;
  char     txt[32];
  //------------------------------------------------------
public:
  CAxeLine() : CSlot(1) {axe = 0; txt[0] = 0;}
  //------------------------------------------------------
  void      Print(CFuiList *w,U_CHAR ln);
  //------------------------------------------------------
  void      SetAxeText(char *t)     {strncpy(txt,t,32);}
  char     *GetAxeText()            {return txt;}
  void      SetAxeDesc(CSimAxe *a)  {axe = a;}
  CSimAxe  *GetAxeDesc()            {return axe;}
};
//===============================================================================
//  Class CButLine to display button in test windows
//===============================================================================
class CButLine: public CSlot {
  //---ATTRIBUTES ---------------------------------------
  char   ktx[32];
  //---Methods ------------------------------------------
public:
  CButLine() : CSlot(1) {ktx[0] = 0;}
  void         Print(CFuiList *w,U_CHAR ln);
  //-----------------------------------------------------
  inline void  SetKeyText(char *t)  {strncpy(ktx,t,32);}
  inline char *GetKeyText()         {return ktx;}
};
//===============================================================================
//  Class CChkLine to display CheckList lines
//===============================================================================
class CChkLine: public CSlot {
  //---ATTRIBUTES --------------------------------------
  char					mark;                   // Check mark
	U_INT         vlid;										// Validation
	D2R2_ACTION		ActDF;									// Action
  //---METHODS -----------------------------------------
public:
  CChkLine(char nl);
  void          Print(CFuiList *w,U_CHAR ln);
	void          SetAction(Tag a, int p1, int p2) {ActDF.SetAction(a,p1,p2);}
  //----------------------------------------------------
  inline Tag    GetAction()       {return ActDF.actn;}
  inline U_INT  GetPM1()          {return ActDF.pm1;}
  inline U_INT  GetPM2()          {return ActDF.pm2;}
  inline short  GetREP()          {return ActDF.rptn;}
  inline short  GetTIM()          {return ActDF.timr;}
  inline short  GetCKA()          {return ActDF.canb;}
  //----------------------------------------------------
	inline void		StoreAction(D2R2_ACTION &a,U_INT p)	{ActDF = a; vlid=p;}
	//----------------------------------------------------
	inline D2R2_ACTION &Action()		{return ActDF;}
  //----------------------------------------------------
  inline void   SwapMark()        {mark ^= 1;}
  inline void   SetMark()         {mark  = 1;}
  inline void   Unmark()          {mark  = 0;}
	inline void   SetVLID(U_INT p)	{vlid	 = p;}
	inline U_INT	GetVLID()					{return vlid;}
  //----------------------------------------------------
	inline bool		Has(U_INT p)			{return ((p & vlid)!=0);}
  inline bool   HasMark()         {return (mark != 0);}
  inline bool   NotLast()         {return cLIN < (nLIN - 1);}
};
//===============================================================================
//  Class CTgxLine to display a generic texture
//===============================================================================
class CTgxLine: public CSlot {
  //----ATTRIBUTES -----------------------------------
  int   type;
  char  nite;
  //----METHOD ---------------------------------------
public:
  CTgxLine();
  void   SetLabel(int t,char *n);
  //----inline ---------------------------------------
  inline  void  SetType(int t)      {type = t;}
  inline  void  SetNite(char n)     {nite = n;}
  //--------------------------------------------------
  inline  int   GetType()           {return type;}
  inline  int   GetNite()           {return nite;}
  //--------------------------------------------------
  bool    Match(void *k)  {return (*((int*)k) == type);}
};
//===============================================================================
//  Class CObjLine to display a 3D object
//===============================================================================
class CObjLine : public CSlot {
  //---ATTRIBUTE -------------------------------------
  CWobj *wObj;                    // World object
  //----METHOD ---------------------------------------
public:
  CObjLine(CWobj *obj);
 ~CObjLine();
  void   Print(CFuiList *w,U_CHAR ln);
  //--------------------------------------------------
  inline  CWobj  *GetWOBJ()         {return wObj;}

};
//===============================================================================
//  Class CWndLine to display a wind layer
//===============================================================================
class CWndLine : public CSlot {
  //---ATTRIBUTE -------------------------------------
  U_CHAR     sNo;           // Slot number
  C3valSlot *slot;          // Slot pointer
  //--- METHOD ----------------------------------------
public:
  CWndLine();
  void  Print(CFuiList *w,U_CHAR ln);
  void  Title(CFuiList *w);
  //---------------------------------------------------
  inline void SetSlot(U_CHAR n,C3valSlot *s)  {sNo = n; slot = s;}
  //---------------------------------------------------
  inline C3valSlot  *GetSlot()    {return slot;}
  inline U_CHAR      GetSlotNo()  {return sNo;}
};
//==================================================================================
//  CLASS CListBox to interface a list box with scrolling
//==================================================================================
#define LIST_HAS_TITLE 0x01
#define LIST_NOHSCROLL 0x02
#define LIST_DONT_FREE 0x04
#define LIST_USE_MARKS 0x08
//----------------------------------------------------------------
typedef void (*SlotCB)(class CSlot *);
//----------------------------------------------------------------
class CListBox {
   //------------Attributes --------------------------------------
private:
  CFuiWindow *Mother;                   // Mother window
  Tag         ident;                    // Tag of listbox
  CFuiList   *wList;                    // List Box
  U_SHORT     Title;                    // Title line
  //-----------------------------------------------------------
  char  typ;                            // Type of box
  char  Opt;                            // Select option
  char  iFree;                          // 0 No free item
  //-- Slot list ----------------------------------------------
  std::vector<CSlot*> Obj;
  //---New Interface ------------------------------------------
  char    htr;                          // Line height
  int     mLIN;                         // Maximum line
  int     eLIN;                         // Last selectable line
  int     sLIN;                         // start line (0/1)= f(Title)
  int     zLIN;                         // Last line
  int     aLIN;                         // Selected line
  int     bLIN;                         // Last selected
  int     hNOD;                         // Head node 
  int     eNOD;                         // Last node to display
  int     nNOD;                         // Total nodes + title
  int     nCAP;                         // Node capacity
  //-------------------------------------------------------------
  U_INT   Num;                          // Sequence number
  //--- Current selection --------------------------------------
  CSlot  *pNOD;                         // Primary node
  int     sTop;                         // Top slot
  int     sBot;                         // Bottom slot
  int     nINS;                         // Insert point
  //------------------------------------------------------------
  //------------------------------------------------------------
private:
  void    InitP1();
  void    InitP2();
  void    PrintFocus();
  void    PrintPage();
  void    PrintTitle();
  void    SetFocus(int lin);
  void    DecFocus();
  void    IncFocus();
  void    SwapFocus(int lin);
  void    NextLine();
  void    PrevLine();
  void    NextPage();
  void    PrevPage();
  void    GotoRatio(int pm);
  void    ShowPage(int lin);
  float   GetRatio(int num, int ttn);
  //-------------------------------------------------------------
  int     SwapNextNode(int No);
  int     SwapPrevNode(int No);
  void    Shift(int fr,int to);
  int     MoveToEnd(int fr);
  int     MoveToTop(int fr);
  int     GetInsertPoint(int cln);
  void    EndInsert();
  void    Renum(int n0, int n1);
  //-----------------Method ------------------------------------
public:
   CListBox();
  virtual	~CListBox();
  void    AddSlot(CSlot *slot);
  void    Apply(SlotCB fn);
  void    SetParameters(CFuiWindow *win,Tag idn,U_INT tp,short lh = 0);
	void		SetProperties(U_INT p,U_INT col);
  void    Reset();
  bool    VScrollHandler(U_INT pm,EFuiEvents evn);
  CSlot  *Search(void *key);
  void    Resize();
	//--- Change properties ----------------------------------------
	void	  SetTransparentMode();		
	void    SetTextColor(U_INT col)	{if (wList) wList->SetColour(col);}
	//---new External interface -----------------------------------
  void    MoveUpItem();
  void    MoveDwItem();
  void    DeleteItem();
  void    InsSlot(CSlot *nod);
  //--------------------------------------------------------------
  void    SortAndDisplay();
  void    Display();
  void    EmptyIt();
	void		Trace();
  //----NEW INTERFACE --------------------------------------------
  void    GoToKey(void *key);
	void		GoToItem(int No);
  CSlot  *GetPrimary();
  U_INT   GetSelectedNo();
  CSlot  *HeadPrimary();
  CSlot  *LastPrimary();
  CSlot  *NextPrimary(CSlot *s);
  CSlot  *PrevPrimary(CSlot *s);
  CSlot  *NextPrimToSelected();
  //-------------------------------------------------------------
  bool    IsTopNode(int No);
  bool    IsLastNode(int No);
  //--------------------------------------------------------------
  int     GetRealSelectedNo();
  //-------------------------------------------------------------
  inline  int GetTopSlot()  {return hNOD + aLIN - sLIN;}
  //-------------------------------------------------------------
  inline void   Refresh()           {if (Mother) PrintPage();}
  inline void   LineRefresh()       {PrintFocus();}
  //-------------------------------------------------------------
  inline U_INT  GetSize()           {return Obj.size();}
  inline void   Clear()             {Obj.clear();}
  inline bool   IsEmpty()           {return (Title >= Obj.size());}
  inline bool   OutSide(U_INT No)   {return (No >= Obj.size());} 
	inline void   NoTitle()						{Title = 0;}
  //-------------------------------------------------------------
public:
  CSlot  *GetSelectedSlot(); 
  CSlot  *GetSlot(U_INT No)   {return (No < Obj.size())?(Obj[No]):(0);}

};
//==================================================================================
#define WAPT_MENU_SIZE (17)
//==================================================================================
//  CFuiRwyEXT to draw runway
//==================================================================================
class CFuiRwyEXT{
protected:
	//--- Drawing attributes --------------------------------------------
	float      scale;												// Map scale                     
  short       wx;                         // Canva mid dimension
  short       wy;                         // 
 	//--- Runway parameters  ---------------------------------------------
  int       lExt;                         // Left extremity   (in nautical miles)
  int       rExt;                         // Right extremity  (dito)
  int       uExt;                         // top extremity    (dito)
  int       bExt;                         // Bottom extremity (dito)
  SPosition oRWY;                         // Origin for position (first runway);
  int         mx;                         // Point of view
  int         my;                         // (dito)
	//-------------------------------------------------------------------
  CFuiCanva	     *grh;										// Graph windows
	CFuiPopupMenu	*ptko;										// List of runways for take off
	CFuiPopupMenu	*plnd;										// List of runways for landing
	//-------------Runway ends management ---------------------------
	CFuiGroupBox  *rend;                    // runway end box
  FL_MENU        men1;                    // Take off menu
	FL_MENU				 men2;										// Landing menu
	char          *cMENU[WAPT_MENU_SIZE];	  // item descriptor (char*)
	void          *pMENU[WAPT_MENU_SIZE];		// Item related
	//-------------------------------------------------------------------
	char       *tkoID;											// take off ID
	char       *lndID;											// Land ID
	char        ilsTXT[20];									// Landing ils
	//-------------------------------------------------------------------
	RWEND			 *tEND;												// Selected take-off end
	RWEND			 *lEND;												// Selected landing end
	float			  ilsFQ;											// Ils frequency
	//-------------------------------------------------------------------
  CListBox		rwyBOX;											// Runway list
	//--- Methods -------------------------------------------------------
public:
	CFuiRwyEXT();
	void	Init();
	void	StoreExtremities(short dx, short dy);
	void	ComputeScale();
	void	ScaleAllRWY();
	int   GetRWYspan();
	void	DrawRunways();
	//--- Runway end points --------------------------------------------
	void	InitRunwayEnds();
};
//==================================================================================
//  CLASS Airport Detail to edit Airport parameters
//==================================================================================
//==================================================================================
class CFuiAptDetail: public CFuiRwyEXT, public CFuiWindow
{ //--------------Attributes --------------------------------------
protected:
  U_CHAR    vers;                         // version 
  CObjPtr   Po;                           // Object smart pointer
  CAirport *Apt;                          // Airport description
  U_SHORT   type;                         // Type APT
  U_SHORT   wptNo;                        // WayPoint number
  SMessage mesg;                          // Tune message
  //-------------Database Request ---------------------------------
  CDataBaseREQ  Req;
  //-------------Comm management ----------------------------------
  CListBox comBOX;                        // Com list box
  //---------------------------------------------------------------
  CFuiCheckbox  *chk;                     // Checkbox
  //-----LIGHT PROFILE MANAGEMENT ---------------------------------------
  CRunway       *cRwy;                    // Current runway in modification
  CRLP          *cRpf;                    // Runway profile
  U_CHAR         lock;                    // Lock state
  CFuiGroupBox  *lpBox;                   // Light profile box
  CFuiPopupMenu *lkPP;                    // Lock popup
  FL_MENU        lkMEN;                   // Menu
  CFuiTextField *hiED;                    // Hi end
  CFuiPopupMenu *hiAP;                    // Hi APPROACH
  FL_MENU        hiAPM;                   // Menu
  CFuiPopupMenu *hiTB;                    // Hi THRESHOLD
  FL_MENU        hiTBM;                   // Menu
  CFuiPopupMenu *hiWB;                    // Hi WING BAR
  FL_MENU        hiWBM;                   // Menu
  CFuiPopupMenu *hiCT;                    // Hi CENTER
  FL_MENU        hiCTM;                   // Menu
  CFuiPopupMenu *hiEG;                    // Hi EDGE
  FL_MENU        hiEGM;                   // Menu
  CFuiPopupMenu *hiTZ;                    // Hi EDGE
  FL_MENU        hiTZM;                   // Menu
  CFuiTextField *loED;                    // Low end
  CFuiPopupMenu *loAP;                    // Lo APPROACH
  FL_MENU        loAPM;                   // Menu
  CFuiPopupMenu *loTB;                    // Lo THRESHOLD
  FL_MENU        loTBM;                   // Menu
  CFuiPopupMenu *loWB;                    // Lo WING BAR
  FL_MENU        loWBM;                   // Menu
  CFuiPopupMenu *loCT;                    // lo CENTER
  FL_MENU        loCTM;                   // Menu
  CFuiPopupMenu *loEG;                    // Lo EDGE
  FL_MENU        loEGM;                   // Menu
  CFuiPopupMenu *loTZ;                    // Hi EDGE
  FL_MENU        loTZM;                   // Menu
  //-------------Methods ------------------------------------------
public:
  CFuiAptDetail(Tag idn, const char *filename,int lim);
  virtual ~CFuiAptDetail();
          void  Initialize(CmHead*obj,U_SHORT type,U_SHORT No);
          void  Draw();
  //-------------Database management     --------------------------
   void   EndOfRequest(CDataBaseREQ *req);
   void   GetAllCOM();
   void   GetRunway();
   void   Terminate();
  //----Light profile ----------------------------------------------
  void    InitLightProfile();
  void    DrawProfile();
  void    LockAll();
  void    UnlockAll();
  void    ChangeProfile(CFuiPopupMenu &pop,int No);
  //----inline methods ---------------------------------------------
  inline  void     SetWayPointNo(U_SHORT No) {wptNo  = No;}
  inline  U_SHORT  GetWayPointNo()           {return wptNo;}
  //-------------Notifications -------------------------------------
  void  NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  void  NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
  bool  TuneRadioCom();
  //------------Database Management ------------------------------
  void  AddDBrecord(void *rec,DBCODE code);
  //--------------------------------------------------------------
  inline void SetRunwayVersion()  {vers = 1;}
};
//=============================================================================
//  Class CFuiGPS to control the GPS in a dedicated window
//=============================================================================
class CFuiKLN89: public CFuiWindow
{ //------------VECTOR TYPE ------------------------------------------
  typedef void(CFuiKLN89::*DisFN)();               // Display function
  typedef int (CFuiKLN89::*ClkFN)(int mx,int my);  // Click Handler
  //------------ATTRIBUTES -------------------------------------------
  CK89gps  *GPS;                                // GPS subsystem
  CBitmap   Back;                               // GPS background
  CBitmap   Front;                              // GPS front
  //--------Screen parameters ----------------------------------------
  CGaugeClickArea   ca[K89_DIM];                // All click area
  short     xOrg;                               // Display x origin
  short     yOrg;                               // Display y origin
  short     yBas;                               // Y for base
  short     rBase;                              // Divider line
  short     dw;                                 // Display wide
  short     dh;                                 // Display height
  U_SHORT   fMask;                              // Flasher mask
  U_CHAR    scrLN[5];                           // screen Line table
  U_SHORT   undTAB[12];                         // Underline table
  //------------------------------------------------------------------
  U_CHAR    PowST;                              // Power state
  //------------Fonts and Colors -------------------------------------
  short     hCar;                               // Character height
  short     wCar;                               // Character wide
  CMonoFontBMP *mono20;                             // font FONT
  U_INT     amber;
  U_INT     black;
  U_INT     yelow;
  //------------Input field table ------------------------------------
  K89_LETFD         letTAB[12];         // Input field table
  K89_PAGFD         pagTAB[12];         // Simple page fields
  K89_REPFD         repTAB[12];         // Table of rept fields
  U_CHAR            WtyFD;              // Working field type
  U_CHAR            WnoFD;              // Working field number
  //------------HANDLERS ---------------------------------------------
  static  DisFN     DshTAB[4];          // Vectors to display handler
  static  ClkFN     HndTAB[4];          // vectors to Click Handler
  //------------Methods ----------------------------------------------
public:
   CFuiKLN89(Tag idn, const char *filename);
  ~CFuiKLN89();
  void  InitGPS();
  void  SetScreenParameters();
  void  InitArea(int No,short x1, short y1, short x2, short y2,U_CHAR d = 0);
  //-----------Input field initializer -------------------------------
  void  IniLF(char No,char *fd);                // Letter fields
  void  IniRP(char No,int *nm);                 // Repeatable fields
  void  IniPF(char No);                         // Page fields
  //-----------Display handler ---------------------------------------
  void  DrawCDIneedle();                        // Draw CDI needle
  void  DrawLSPDisplay();                       // Left part
  void  DrawRSPDisplay();                       // Right part
  void  DrawAllDisplay();                       // Full display
  void  DrawLDRDisplay();                       // Partial display
  void  DrawGPSwindow();                        // Opening window
  //-----------Mouse handlers ----------------------------------------
  bool  InsideClick (int x, int y, EMouseButton buttons);
  int   AnyNULclick(int mx,int my);
  int   AnyLETclick(int mx, int my);
  int   AnyCHRclick(K89_LETFD *fd,int mx,int my);
  int   AnyGENclick(int mx,int my);
  int   AnyFLDclick(K89_REPFD *fd,int mx,int my);
  int   AnySPFclick(K89_PAGFD *fd,int mx,int my);
  //------------------------------------------------------------------
  bool  MouseStopClick (int x, int y, EMouseButton buttons);
  //-----------Drawing control ---------------------------------------
  void  ClearGPSwindow(U_INT col);
  bool  PowerChange(U_CHAR astate);
  void  Draw();
};
//==========================================================================================
//  WIND layer adjustment
//==========================================================================================
class CFuiWind: public CFuiWindow {
  //------ATTRIBUTES --------------------------------------------------
  CFuiList      *layW;                          // Layer window
  CFuiLabel     *errW;                          // error window
  CFuiPopupMenu *popW;                          // Cloud cover
  CFuiTextField *skyW;                          // Ceil
  //-------------------------------------------------------------------
  FL_MENU        cMEN;                          // Cloud menu
  char          layer;                          // Cloud layer
  //-----Limit to modify altitude -------------------------------------
  float          altd;                          // Lower limit
  float          altu;                          // Upper limit
  //-----Cloud ceil ---------------------------------------------------
  float          ceil;
  //-------------------------------------------------------------------
  CWndLine      *sel;                           // Selected line
  C3valSlot     *val;                           // Selected slot
  //------METHODS -----------------------------------------------------
public:
  CFuiWind(Tag idn, const char *filename);
  void  Error(char *msg);
  void  Select();
  void  ModifyAltitude(float m);
  void  ModifyDirection(float m);
  void  ModifySpeed(float m);
  void  ChangeCover(char c);
  void  ChangeCeil(float m);
  //-------------------------------------------------------------------
  void  Draw();
  //-------------------------------------------------------------------
  void  NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
  void  NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
};
//==========================================================================================
//  Weather overview
//==========================================================================================
class CFuiWeatherView: public CFuiWindow {
  //---- ATTRIBUTES ---------------------------------------------------
  CFuiLabel   *datW;                    // Date
  CFuiLabel   *timW;                    // Time local
  CFuiLabel   *utcW;                    // Time utc
  CFuiLabel   *disW;                    // Distance
  CFuiLabel   *locW;                    // Location
  CFuiLabel   *altW;                    // Altitude
  CFuiLabel   *winW;                    // Wind
  CFuiLabel   *tmpW;                    // Temperature
  CFuiLabel   *barW;                    // Barometer
  CFuiLabel   *cldW;                    // Cloud
  CObjPtr      Apt;                     // Nearest airport
  CDataBaseREQ  Req;                    // Database Request
  //---- Methods ------------------------------------------------------
public:
  CFuiWeatherView(Tag idn, const char *filename);
  ~CFuiWeatherView() {Apt = 0;}
  //-------------------------------------------------------------------
  void    EditDate();
  void    EditTime();
  void    EditLocation();
  void    NoLocation();
  void    EditAltitude();
  void    EditWind();
  void    EditTemp();
  void    EditBarometer();
  void    EditCloud();
  //-------------------------------------------------------------------
  void    Draw();
};
//==========================================================================================
//  AIRCRAFT CHECK LIST
//==========================================================================================
class CFuiCkList: public CFuiWindow {
  //------ATTRIBUTES -------------------------------------------------
  CFuiButton    *uBUT;                  // Uncheck Button
  CFuiButton    *wBUT;                  // Where   Button
  CFuiButton    *dBUT;                  // Done button
  CFuiPopupMenu *cPOP;                  // Chapter POPUP
  FL_MENU        mPOP;                  // Chapter menu
  //------------------------------------------------------------------
  int            nSEL;                  // Current selection
  //-------------------------------------------------------------------
  CListBox       iBOX;
  //---CHECKLIST -----------------------------------------------------
  PlaneCheckList      *LST;
  //------------------------------------------------------------------
  CChkLine     *prev;                   // previous slot
  CChkLine     *slot;                   // Selected slot
  CRobot       *robot;                  // Robot location
  SMessage      msg;                    // Locate actionner
  CPanel       *panl;										// Selected panel
  CGauge       *gage;                   // Gauge to act uppon
  Tag           tgag;                   // Gauge tag
  //-----Action parameters -------------------------------------------
  Tag           actn;                   // Action
  int           p1;                     // Parameter 1
  int           p2;                     // Parameter 2
  int           ca;                     // Click area
	//---Camera ----------------------------------------
	CCamera      *cam;										// Current camera
  //------METHODS ----------------------------------------------------
public:
  CFuiCkList(Tag idn, const char *filename);
 ~CFuiCkList();
  //------------------------------------------------------------------
  void  BuildMenu();
  void  NewChapter(int No);
  void  LocateGage();
  void  StopBlink();
  bool  Verify();
  void  Execute();
  void  EndExecute(void *lin);
  //-----NOTIFICATIONS -----------------------------------------------
  void  NotifyFromPopup(Tag id,Tag itm,EFuiEvents evn);
  void  NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
  };
//==========================================================================================
//  STRIP OF TEXT
//==========================================================================================
class CFuiStrip: public CFuiWindow {
  //------- ATTRIBUTES ----------------------------------------------------
  CFuiTextField *iBOX;
  CFuiScrollBTN *lBOX;
  CFuiScrollBTN *rBOX;
  char Iden[6];
  //--------METHODS -------------------------------------------------------
public:
  CFuiStrip(Tag idn, const char *filename);
  void    SetIdent(char *iden);
  void    NoMetar();
  void    GetMetar(char *idn);
  //------NOTIFICATIONS ---------------------------------------------------
  void    NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
};

//==========================================================================================
//  TERRA BROWSER
//==========================================================================================
class CTerraFile;
class CFuiTBROS: public CFuiWindow {
  //------- ATTRIBUTES ---------------------------------------------
	CRabbitCamera   *rcam;												// Rabbit camera
	//-----------------------------------------------------------------
  CFuiList        *xWIN;                        // List textures
  CListBox        *aBOX;                        // Driving box
  CFuiLabel       *wLB1;                        // Label 1
  CFuiCanva       *xCNV;                        // Texture Canva
  CFuiCheckbox    *wOPT;                        // Option
  CFuiButton      *wBTN;                        // Assign button
  CFuiButton      *zBTN;                        // Cancel button
  //---Original context -----------------------------------------
  CAMERA_CTX       ctx;                         // Original camera
  //-----------------------------------------------------------------
  CTerraFile      *tFIL;                        // Tile file 
  //-----------------------------------------------------------------
  char           lock;                          // Lock mode
  char           mode;                          // Auto mode 
  int            Type;                          // Texture type
  //----Changed tile ------------------------------------------------
  U_INT            sx;                          // Absolute indice
  U_INT            sz;                          // Absolute indice
  char           chng;                          // Changed type
  TEXT_INFO      inf;                           // Texture info
  CCamera       *Cam;                           // Any camera
  //-----------------------------------------------------------------
public:
  CFuiTBROS(Tag idn, const char *filename);
 ~CFuiTBROS();
  //------------------------------------------------------------------
  int     Error(); 
  int     RestoreTile(int gnd);
  int     ChangeTile(int gnd);
  void    Draw();
  void    RefreshOPT();
  void    UpdateGround();
  void    GetSelection();
  void    NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
	//--- Dont close this window ----------------------------------------
	bool    CheckProfile(char a);
};
//==========================================================================================
//  OBJECT BROWSER
//==========================================================================================
class CFuiMBROS: public CFuiWindow {
    //--- Attribut -------------------------------------------------
		CRabbitCamera   *rcam;												// Rabbit camera
		//--------------------------------------------------------------
    CFuiLabel       *mPOS;                        // Model position
    CFuiLabel       *mNIT;                        // Nite Model
    CFuiCanva       *oDSP;                        // Object Canva
    CFuiSlider      *zSLD;                        // ZOOM slider
    CFuiLabel       *zLAB;                        // ZOOM label
    CFuiList        *oWIN;                        // List object
    CFuiCanva       *oINF;                        // Object Canva
    CListBox         oBOX;                        // object box
    CFuiLabel       *wTOT;                        // Total object
		CFuiLabel       *wOrg;												// Object origin
		CFuiCheckBox    *wZbo;												// ZBuffer option
    //---Original position -----------------------------------------
    CAMERA_CTX       ctx;                         // Original camera
    //---MODEL PARAMETERS ------------------------------------------
    CObjLine        *Lin;                         // Current line
    CWobj           *wObj;                        // Selected Object
    C3Dmodel        *Mod;                         // Current model
    //---Camera Parameters -----------------------------------------
    CCameraObject   *oCam;
    CCameraOrbit    *sCam;
    U_INT            Prof;                      // Profile
    //--------------------------------------------------------------
public:
    CFuiMBROS(Tag idn, const char *filename);
   ~CFuiMBROS();
    //--------------------------------------------------------------
    void  NewSelection();
    void  Teleport();
    //-----------------------------------------------------------------
    void  NotifyChildEvent(Tag idm,Tag itm,EFuiEvents evn);
    void  ChangeZoom();
    void  SetZoom();
		void	ChangeZB();
    //----------------------------------------------------------------
    void  Draw();
    void  DrawByCamera(CCamera *cam);
		//--- Dont close this window ----------------------------------------
	  bool  CheckProfile(char a);
};
//==================================================================================
//  CFuiDetail.  A generic fui window for VOR and airport
//==================================================================================
class CFuiDetail {
	// Attributes ----------------------------------------------------
	//--- Methods ----------------------------------------------------
public:
	CFuiDetail();
 ~CFuiDetail();
  //----------------------------------------------------------------
  bool        CreateVORwindow(CmHead *obj,U_INT No,int lim);
  bool        CreateNDBwindow(CmHead *obj,U_INT No,int lim);
  bool        CreateAPTwindow(CmHead *obj,U_INT No,int lim);
  bool        CreateAPTwinLIT(CmHead *obj,U_INT No,int lim);
  bool        SmallDetailObject(CmHead *obj,U_INT No);
  bool        OpenWinDET (CmHead *obj,U_INT No);
};

#endif // FUIPART_H
//====================END OF FILE ==========================================================