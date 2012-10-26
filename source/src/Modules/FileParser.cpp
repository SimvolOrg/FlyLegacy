/*
 * File Parser
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
#include "../Include/FlyLegacy.h"
#include "../Include/Globals.h"
#include "../Include/TerrainCache.h"
#include "../Include/TerrainUnits.h"
#include "../Include/TerrainTexture.h"
#include "../Include/Triangulator.h"
#include "../Include/Model3D.h"
#include "../Include/Endian.h"
#include "../Include/FileParser.h"
#include "../Include/Airport.h"
#include "../Include/Weather.h"
#include <stdio.h>
#include <io.h>

//=============================================================================
//  METAR QUALIFIER
//=============================================================================
Tag qalMETAR[] = 
{ 'MI',               // SHALLOW
  'BC',               // PATCHES
  'DR',               // Drifting
  'BL',               // Blowing
  'SH',               // SHOWER
  'TS',               // Thunderstorm
  'FZ',               // Freezing
  'PR',               // PARTIAL
     0,               // End table
};
//=============================================================================
//  METAR WEATHER TYPE
//=============================================================================
Tag wthMETAR[] =
{ 'RA',               // Rain
  'SN',               // Snow
  'FG',               // Fog
  'BR',               // Mist
  'HZ',               // Haze
  'GR',               // Hail
  'SA',               // Sand
  'DZ',               // Drizzle
  'SQ',               // Squalls
  'GS',               // Small hail
  'IC',               // Diamond dust
  'PL',               // Ice pellet
  'SS',               // Sand storm
  'FU',               // Smoke
  'VA',               // Volcanic ash
  'DU',               // Widespray dust
  'DS',               // Dust storm
  'FC',               // Funnel cloud
     0,               // End table
};
//=============================================================================
//  METAR CLOUD COVER
//=============================================================================
Tag covMETAR[] =
{ 'CLR',              // 0-Clear below 12000 feet
  'SKC',              // 1-Sky Clear
  'NSC',              // 2-No cloud above 5000 feet
  'FEW',              // 3-Layer 2    1/8 2/8
  'SCT',              // 4-Scattered  3/8 4/8
  'BKN',              // 5-Broken     5/8 6/8
  'OVC',              // 6-Overcast 
      0,              // End table
};
//=============================================================================
//  METAR CLOUD Layer
//=============================================================================
int layMETAR[] = {
  0,                // CLR
  0,                // SKC
  1,                // NSC
  2,                // FEW
  3,                // SCT
  4,                // BKN
  6,                // OVC
};
//=============================================================================
//  Indices to decompose a QUAD in triangles
//=============================================================================
int quadIND[] = {0,1,2,0,2,3};
//========================================================================
//	Common file parser
//	A parser will build a list of C3DParts
//========================================================================
CParser::CParser(char t)
{	part		= 0;
	retcode = M3D_LOADED;
	pod			= 0;
	type		= t;
	tREF		= 0;
	Dir			= 0;
	vmin.x	= vmin.y = vmin.z = 0;
  vmax		= vmin;
 *txname  = 0;
}
//----------------------------------------------------------------------
//  Save Model extension
//----------------------------------------------------------------------
void CParser::SaveExtension(F3_VERTEX &vt)
{ if (vt.VT_X < vmin.x)  vmin.x = vt.VT_X;
  if (vt.VT_X > vmax.x)  vmax.x = vt.VT_X;
  if (vt.VT_Y < vmin.y)  vmin.y = vt.VT_Y;
  if (vt.VT_Y > vmax.y)  vmax.y = vt.VT_Y;
  if (vt.VT_Z < vmin.z)  vmin.z = vt.VT_Z;
  if (vt.VT_Z > vmax.z)  vmax.z = vt.VT_Z;
  return;
}
//----------------------------------------------------------------------
//  Save Model extension
//----------------------------------------------------------------------
void CParser::SaveExtension(TC_VTAB &v)
{	if (v.VT_X < vmin.x)  vmin.x = v.VT_X;
  if (v.VT_X > vmax.x)  vmax.x = v.VT_X;
  if (v.VT_Y < vmin.y)  vmin.y = v.VT_Y;
  if (v.VT_Y > vmax.y)  vmax.y = v.VT_Y;
  if (v.VT_Z < vmin.z)  vmin.z = v.VT_Z;
  if (v.VT_Z > vmax.z)  vmax.z = v.VT_Z;
	return;
}
//----------------------------------------------------------------------
//  Save Model extension
//----------------------------------------------------------------------
void CParser::SaveExtension(GN_VTAB &v)
{	if (v.VT_X < vmin.x)  vmin.x = v.VT_X;
  if (v.VT_X > vmax.x)  vmax.x = v.VT_X;
  if (v.VT_Y < vmin.y)  vmin.y = v.VT_Y;
  if (v.VT_Y > vmax.y)  vmax.y = v.VT_Y;
  if (v.VT_Z < vmin.z)  vmin.z = v.VT_Z;
  if (v.VT_Z > vmax.z)  vmax.z = v.VT_Z;
	return;
}

//----------------------------------------------------------------------
// Get a statement
//----------------------------------------------------------------------
int CParser::GetStatement(char *s)
{	bool go = true;
	while (go)
	{ char *B = pgets(s,256,pod);
		if (0 == B)			return 0;
		if (*B == 0x0A)	continue;
		return 1;
	}
	return 0;
}
//---------------------------------------------------------------------
// Load a 3D model with the part Queue
//---------------------------------------------------------------------
int  CParser::LoadModel(C3Dmodel *mod)
{	mod->SetExtend(vmin,vmax);
  mod->SaveTop(vmax.z);
	mod->SaveBot(vmin.z);
	C3DPart *prt = 0;
	for (prt = partQ.Pop(); prt != 0; prt = partQ.Pop()) 	mod->AddPodPart(prt);
	return retcode;
}
//---------------------------------------------------------------------
//  Return a warning for file
//---------------------------------------------------------------------
int CParser::StopParse(PODFILE *p,char *msg)
{ char *fn = (p)?(p->filename):("");
  SCENE("*** File Parsed %s error: %s", fn,msg);
  if (part)  delete part;
  part  = 0;
	partQ.Clear();
  retcode = M3D_ERROR;
  if (pod)  pclose(pod);
	pod		= 0;
  return  M3D_ERROR;
}
//---------------------------------------------------------------------
//  Return a warning for file
//---------------------------------------------------------------------
C3DPart *CParser::GetOnlyOnePart()
{	C3DPart *p0 = partQ.Pop();
	partQ.Clear();
	return p0;
}
//==================================================================================
//  TIF PARSER
//  inspired by C.Wallace ImageTIFF.cpp
//==================================================================================
CTIFFparser::CTIFFparser(U_CHAR opt)
{ Name  = 0;
  Wd    = 0;
  Ht    = 0;
  Dp    = 0;
  Tsp   = opt;
  Big   = 0;
  Fmt   = 0;
  Bps   = 0;
  Strip = 0;
  cMap  = 0;
  Nst   = 0;
  nPix  = 0;
  rgba  = 0;
  CMdim = 0;
}
//----------------------------------------------------------------------------
//  Warning
//----------------------------------------------------------------------------
int CTIFFparser::Warning(PODFILE *p,char *msg)
{ if (p)    pclose (p);
  WARNINGLOG("TIFparser file: %s (%s)",Name,msg);
  return 0;
}
//----------------------------------------------------------------------------
//  Destroye Parser
//----------------------------------------------------------------------------
CTIFFparser::~CTIFFparser()
{ if (Strip)  delete [] Strip;
  if (cMap)   delete [] cMap;
  if (rgba)   delete [] rgba;
}
//----------------------------------------------------------------------------
// Read a short data from the TIF stream.  
//-----------------------------------------------------------------------------
short CTIFFparser::Read16 (PODFILE *p)
{ short s;
  pread (&s, sizeof(short), 1, p);
  return (Big)?(BigEndian(s)):(LittleEndian(s));
}
//-----------------------------------------------------------------------------
//  Read a long int fromm TIF stream
//-----------------------------------------------------------------------------
long CTIFFparser::Read32 (PODFILE *p)
{ long v;
  int nr = pread (&v, sizeof(long), 1, p);
  if (1 != nr)     gtfo("Bad file");
  return (Big)?(BigEndian(v)):(LittleEndian(v));
}
//----------------------------------------------------------------------------
//  Read Short from memory
//----------------------------------------------------------------------------
U_SHORT CTIFFparser::Value(short s)
{ return (U_SHORT)(Big)?(BigEndian(s)):(LittleEndian(s));  }
//----------------------------------------------------------------------------
//  Process Image format
//----------------------------------------------------------------------------
int CTIFFparser::GetFormat(PODFILE *p,int fmt)
{ switch (fmt)  {
        case 0:
        // Monochrome, 0 is White
        return Warning(p,"Monochrome not supported");

      case 1:
        // Monochrome, 0 is Black
        return Warning(p,"Monochrome not supported");

      case 2:
        // RGB
        Fmt = TIF_RGBA;
        return 1;

      case 3:
        // Palette color
        Fmt = TIF_CMAP;
        return 1;

      case 4:
        // Transparency mask
        return Warning(p,"Transparency not supported");
  }
  return 0;
}
//----------------------------------------------------------------------------
//  Read Strip offset
//----------------------------------------------------------------------------
int CTIFFparser::GetStripOffset(PODFILE *p)
{ Nst   = Dir.count;
  Strip = new TIF_STRIP[Nst];
  if (1 == Dir.count)
  { Strip[0].ofs = Dir.offset;
    Strip[0].cnt = 0;
    return 1;
  }
  //-----Read all offsets -------------------------------
  long tell = ptell (p);
  //--- Set file pointer to read strip offset data
  pseek (p, Dir.offset, SEEK_SET);
  switch (Dir.type) {
    case TIF_TYPE_SHORT:
      // Offsets are short integers, must be read one at a time and converted to long
      for (U_INT j=0; j<Dir.count; j++) Strip[j].ofs = (unsigned long)Read16 (p);
      break;
    case TIF_TYPE_LONG:
      // Offsets are long integers
      for (U_INT j=0; j<Dir.count; j++) Strip[j].ofs = (unsigned long)Read32 (p);
      break;
    default:
      return Warning(p,"Bad Strip type");
  }
  //----Restore file position ----------------------------
  pseek (p, tell, SEEK_SET);
  return 1;
}
//----------------------------------------------------------------------------
//  Read Strip count
//----------------------------------------------------------------------------
int CTIFFparser::GetStripCount(PODFILE *p)
{ if (0 == Strip) return Warning(p,"No Strip");
  if (1 == Dir.count)
  { Strip[0].cnt = Dir.offset;
    return 1;
  }
  //-----Read all counts -------------------------------
  long tell = ptell (p);
  //--- Set file pointer to read strip offset data
  pseek (p, Dir.offset, SEEK_SET);
  switch (Dir.type) {
    case TIF_TYPE_SHORT:
      // Offsets are short integers, must be read one at a time and converted to long
      for (U_INT j=0; j<Dir.count; j++) Strip[j].cnt = (unsigned long)Read16 (p);
      break;
    case TIF_TYPE_LONG:
      // Offsets are long integers
      for (U_INT j=0; j<Dir.count; j++) Strip[j].cnt = (unsigned long)Read32 (p);
      break;
    default:
      return Warning(p,"Bad Strip type");
  }
  //----Restore file position ----------------------------
  pseek (p, tell, SEEK_SET);
  return 1;
}

//----------------------------------------------------------------------------
// Read the ColorMap
// The size is 3 times the size of the number of color. 
//----------------------------------------------------------------------------
void CTIFFparser::GetColorMAP(PODFILE *p)
{ U_INT nr = 0;
  if (0 == CMdim)       return;
  short  *buf = new short[CMdim];
  short  *src = 0;
  cMap        = new U_INT[CMdim];
  //--Get data from file --------------------------------
  pseek (p, CMofs, SEEK_SET);
  //---- First come all red values ----------------------
  nr = pread(buf,sizeof(short),CMdim,p);
  if (nr != CMdim)  gtfo("Bad file");
  src = buf;
  for (U_INT k=0; k < CMdim; k++)
  { U_CHAR red = (Value(*src++) >> 8) & 0xFF;
    cMap[k]    = red;
  }
  //----- Next all green values -------------------------
  nr = pread(buf,sizeof(short),CMdim,p);
  if (nr != CMdim)  gtfo("Bad file");
  src = buf;
  for (U_INT k=0; k < CMdim; k++) 
  { U_CHAR grn = (Value(*src++) >> 8) & 0xFF;
    cMap[k]    |= (grn <<  8);
  }
  //----- Next come all blue values  -------------------
  nr = pread(buf,sizeof(short),CMdim,p);
  if (nr != CMdim)  gtfo("Bad file");
  src = buf;
  for (U_INT k=0; k < CMdim; k++)
  { U_CHAR blu = (Value(*src++) >> 8)  & 0xFF;
    cMap[k]    |= (blu << 16);
  }
  delete buf;
  return;
}

//----------------------------------------------------------------------------
//  Get Image in memory
//----------------------------------------------------------------------------
void CTIFFparser::GetStripFromCMAP(PODFILE *p,int k,U_INT *dst)
{ int     nr  = 0;
  int     dim = Strip[k].cnt;
  int     ofs = Strip[k].ofs;
  U_INT   alf = 0;
  U_CHAR *buf = new U_CHAR[dim];
  pseek (p, ofs, SEEK_SET);
  nr = pread (buf, sizeof(U_CHAR), dim, p);
  if (nr != dim)    gtfo("bad file");
  U_CHAR *src = buf;
	U_CHAR *end = buf + nr;											// End of source buffer
  for (U_INT q=0; q < Rps; q++)               // For each row in strip
    for (U_INT m=0; m < Wd; m++)              // For each pixel in a row
    { if (src  >= end)				break;					// Stop reading
			if (cnbp >= nPix)				break;					// Stop writing
			U_INT ind = *src++;                     // Get pixel index
      U_INT pix = cMap[ind];                  // RGB color
      if (Spp == 2) alf = ((*src++) << 24);   // Alpha chanel
      else          alf = ((pix == 0))?(Tsp << 24):(0xFF000000);    // Full Alpha
      pix   |= alf;                           // Add alpha chanel
     *dst++  = pix;                           // Save pixel
    }
  bdst  = dst;                                // Save destination
  delete [] buf;
  return;
}
//----------------------------------------------------------------------------
//  Get Image from RGBA
//  
//----------------------------------------------------------------------------
void CTIFFparser::GetStripFromRGBA(PODFILE *p,int k,U_INT *dst)
{ int     nr = 0;
  int     dim = Strip[k].cnt;
  int     ofs = Strip[k].ofs;
  U_INT   alf = (U_INT)(0xFF << 24);
  U_CHAR *buf = new U_CHAR[dim];
  pseek (p, ofs, SEEK_SET);
  nr = pread (buf, sizeof(U_CHAR), dim, p);
  U_CHAR *src = buf;
  U_INT   pix = 0;
	//--- put some caution not to go over source ---------------
	U_CHAR *end = buf + nr;											// End of source buffer
  for (U_INT q=0; q < Rps; q++)               // For each row in strip
    for (U_INT m=0; m < Wd; m++)              // For each pixel in a row
    { if (src >= end)		break;								// Stop reading
			if (cnbp >= nPix)	break;								// Stop writing
			pix  = (*src++);                        // Red component
      pix |= (*src++ << 8);                   // Green component
      pix |= (*src++ << 16);                  // Blue component
      if (Spp == 4)   pix |= (*src++ << 24);  // Alpha chanel
      else            pix |= alf;             // Full alpha
     *dst++  = pix;                           // Store pixel
		  cnbp++;																	// One more pixel
    }
  bdst = dst;
  delete [] buf;
  return;
}
//----------------------------------------------------------------------------
//  Transfert RGBA responsibility
//----------------------------------------------------------------------------
GLubyte *CTIFFparser::TransferRGB()
{ GLubyte *rgb = (GLubyte*)rgba;
  rgba  = 0;
  return rgb;
}
//----------------------------------------------------------------------------
//  Transfert RGBA responsibility
//----------------------------------------------------------------------------
U_INT *CTIFFparser::TransferPIX()
{ U_INT *rgb = rgba;
  rgba  = 0;
  return rgb;
}
//----------------------------------------------------------------------------
//  Invert image verticaly
//----------------------------------------------------------------------------
void CTIFFparser::InvertPIX()
{ U_INT *rgb = new U_INT [nPix];
  U_INT *src = rgba;
  U_INT *dst = rgb + nPix - Wd;
  for (U_INT ln=0; ln < Ht; ln++)
  { CopyLine(src,dst);
    src += Wd;
    dst -= Wd;
  }
  delete [] rgba;
  rgba  = rgb;
  return;
}
//----------------------------------------------------------------------------
//  Transfer the line
//----------------------------------------------------------------------------
void CTIFFparser::CopyLine(U_INT *src,U_INT *dst)
{ for (U_INT k = 0; k < Wd; k++)  *dst++ = *src++;
  return;
}
//----------------------------------------------------------------------------
//  BUILD IMAGE RGBA
//----------------------------------------------------------------------------
int CTIFFparser::GetImage(PODFILE *p)
{ bdst = rgba;
	cnbp = 0;
  if (Fmt == TIF_CMAP)
  { for (U_INT k = 0; k < Nst; k++)   GetStripFromCMAP(p,k,bdst);
    return 1;
  }
  if (Fmt == TIF_RGBA)
  { for (U_INT k = 0; k < Nst; k++)   GetStripFromRGBA(p,k,bdst);
  return 1;
  }
  Warning(p,"Not decoded");
  return 0;
}
//----------------------------------------------------------------------------
//  Decode the file
//----------------------------------------------------------------------------
int  CTIFFparser::Decode(char *fn)
{ // Open POD file
  Name  = fn;
  PODFILE *p = popen (&globals->pfs, Name);
  if (0 == p)                         return Warning(p,"Can't open");
  pread (&Hdr, sizeof(TIF_HDR), 1, p);
  if (PTIF_LITTLE_ENDIAN == Hdr.endian)
  {   Hdr.signature = LittleEndian (Hdr.signature);
      Hdr.ifd       = LittleEndian (Hdr.ifd);
  }
  if (PTIF_BIG_ENDIAN    == Hdr.endian)
  {   Big = 1;  
      Hdr.signature = BigEndian (Hdr.signature);
      Hdr.ifd       = BigEndian (Hdr.ifd);
  }
  //----Check signature -----------------------------------------------
  if (Hdr.signature != PTIF_SIGNATURE) return Warning(p,"Bad signature");
  //------ Read number of directory entries ----------------------------
  pseek (p, Hdr.ifd, SEEK_SET);
  unsigned short nEntries = (unsigned short) Read16 (p);
  //-------Read directory entries --------------------------------------
  for (int k=0; k<nEntries; k++) {
    Dir.tag    = Read16 (p);
    Dir.type   = Read16 (p);
    Dir.count  = Read32 (p);
    Dir.offset = Read32 (p);
	//TRACE("Tag %u Type %u Count %u Offset %u",Dir.tag, Dir.type, Dir.count, Dir.offset);
    //-------PROCESS TAG -------------------------------------------------
    switch (Dir.tag) {
      //----Bitmap width in pixels --------------------
      case TIF_TAG_IMAGEWIDTH:
        Wd = Dir.offset;
        continue;
      //---Bitmap lenght ------------------------------
      case TIF_TAG_IMAGELENGTH:
        Ht = Dir.offset;
        continue;
      //---BIT PER SAMPLE -----------------------------
      case TIF_TAG_BITSPERSAMPLE:
        Bps = Dir.offset;
        continue;
      //---BIT per PIXEL -------------------------------
      case TIF_TAG_SAMPLESPERPIXEL:
        Spp = Dir.offset;
        continue;
      //---Compressed not supprted ---------------------
      case TIF_TAG_COMPRESSION:
        if (Dir.offset == 1)         continue;
        return Warning(p,"Compression not supported");
      //---Decode format --------------------------------
      case TIF_TAG_INTERPRETATION:
        if (GetFormat(p,Dir.offset))  continue;
        return 0;
      //---New SubfileType (ignore) ---------------------
      case TIF_TAG_NEWSUBFILETYPE:
        continue;
      //---STRIP OFFSET ---------------------------------
      case TIF_TAG_STRIPOFFSETS:
        if (GetStripOffset(p))        continue;
        return 0;
      //---ROW PER STRIP --------------------------------
      case TIF_TAG_ROWSPERSTRIP:
        Rps = Dir.offset;
        continue;
      //---STRIP BYTE COUNT ---------------------------
      case TIF_TAG_STRIPBYTECOUNTS:
        if (GetStripCount(p))         continue;
        return 0;
      //---PLANAR CONFIGURATION -------------------------
      case TIF_TAG_PLANARCONFIGURATION:
        if (Dir.offset == 1)          continue;
        //---Other configuration not supported ---------
        return Warning(p,"Planar value not supported");
      //---COLOR MAP ------------------------------------
      case TIF_TAG_COLORMAP:
        CMdim = Dir.count / 3;
        CMofs = Dir.offset;
        continue;
			//--- Tag to ignore -------------------------------

			default:
				continue;
//				gtfo("TIF %s invalid tag %u", fn, Dir.tag);
    }
    }
  //--NOW BUILD THE RGB MAP -------------------------------
  nPix  = (Wd * Ht);
  rgba  = new U_INT[nPix];
  GetColorMAP(p);
  GetImage(p);
  pclose (p);
  return 1;
}
//=============================================================================
//  SMF File decoder
//  NOTES:  All coordinates are supposed to be in feet relative to object origin
//=============================================================================
//---------------------------------------------------------------------
//  Parser for 3Dmodel
//---------------------------------------------------------------------
CSMFparser::CSMFparser(char t)
	:CParser(t)
{ Tsp   = 0xFF;
  Res   = MODEL_HI;
  nFace = 0;
}
//---------------------------------------------------------------------
//  Decode the SMF file
//  -mod indicates weither this is a DAY/NIGHT object
//---------------------------------------------------------------------
int CSMFparser::Decode(char *fn,char t)
{ int nbp     = 0;
  int dtl     = 0;
	type				= t;
  float pix   = 0;
  fname       = fn;
  PODFILE *p = popen (&globals->pfs,fn);
	pod					= p;
  if (0 == p)																			return StopParse(0, "No FILE ");
  char s[256];
  //--- Object type, this should be "C3DModel"
  pgets (s, 256, p);
  if (strncmp (s, "C3DModel",8) != 0)							return StopParse(p,"TYPE");
  //--- Skip Version (ignored)
  pgets (s, 256, p);
  sscanf(s,"%d",&version);
  //--- Part count ------------------------
  pgets (s, 256, p);
  if (sscanf (s, "%d", &nbp) != 1)								return StopParse(p,"COUNT");
  //---- Auto detail ----------------------
  if (version > 3)
  {  pgets (s, 256, p);
     if (sscanf (s, "%d,%f", &dtl, &pix) != 2)		return StopParse(p,"DTL");
  }
  //---- Read parts -----------------------
  for (int k=0; k!= nbp; k++)  if (M3D_ERROR == ReadPart(p,fn)) return retcode;
  if (pod)	pclose(pod);
  return retcode;
}
//----------------------------------------------------------------------
//  Get current part from file
//  Coordinates are transformed 
//  from LHS to RHS by inverting X and Y coordinates
//  NOTE:
//  The C3Dpart  canonical format is
//    A shared vertex list    (V0,V1,...,Vn) where Vk=(x,y,z)
//    A shared normal list    (N0,N1,...,Nn) where Nk=(m,n,p)
//    A shared tex coord list (T0,T1,...,Tn) where Tk=(s,t)
//    A List of indices in above array (I0,I1,.. Ip)
//  
//----------------------------------------------------------------------
int CSMFparser::ReadPart(PODFILE *p,char *fn)
{ int      nf = 0;
  bool     v1 = false;
  char s[256];
  pgets(s, 256, p);
  ReadType(s);
  //--- Part status (on/off) ignored
  pgets(s, 256, p);
  //--- Version (ignored)
  pgets(s, 256, p);
  //--- Vertex count, frame count, face count, unused value
  int nVerts, nFrames, nbfaces, dummy;
  pgets (s, 256, p);
  nf = sscanf (s, "%d,%d,%d,%d", &nVerts, &nFrames, &nbfaces, &dummy);
  if (4 != nf)                  return StopParse(p,"VTX");
  nFace += nbfaces;
  //--- Skip optional v1 flag ---------------------
  pgets (s, 256, p);
  if (strncmp (s, "v1", 2) == 0) {v1 = true; pgets (s, 256, p);}
  //--- Other parameters --------------------------
  float dif, spc, pow;
  int   trn,env;
  
  nf = sscanf (s, "%f,%f,%f,%d,%d, %31s ",&dif, &spc, &pow, &trn, &env, txd.name);
  if (6 != nf)          return StopParse(p,"TEXT");
  // If the line before last was "v1" then skip the next line, which is
  //   always a blank string ""
  //---Convert texture name in upper case --Compute texture type ------
  _strupr_s(txd.name,TC_TEXTURE_NAME_DIM);
  if (v1)  pgets (s, 256, p);
  int   tot = nVerts * nFrames;
	if (nVerts == 0)		return StopParse(p,"No Vertices");
  //----Read vertices ------------------------------------
  //  NOTE: Y and Z are reverted for LHS to RHS conversion
  //
  part             = new C3DPart();
	part->AllocateW3dVTX(tot);
  F3_VERTEX *cvt  = part->GetVLIST();           // Vertex list
  F3_VERTEX *cvn  = part->GetNLIST();           // Normal coordinates
  F2_COORD  *ctu  = part->GetTLIST();           // Texture coordinates
  for (int k=0; k<nVerts; k++)
  { pgets (s, 256, p);
    nf =  sscanf (s, "%f,%f,%f,%f,%f,%f,%f,%f",
            &cvt->VT_X,&cvt->VT_Z,&cvt->VT_Y,
            &cvn->VT_X,&cvn->VT_Z,&cvn->VT_Y,
            &ctu->VT_S,&ctu->VT_T);
    if (8 != nf)      return StopParse(p,"VTX");
    //---Adjust coordinates here -------------------------------
    ctu->VT_T = 1 + ctu->VT_T;
    //---Want to save model extension here from ent ------------
    SaveExtension(*cvt);
    //---Next component ----------------------------------------
    cvt++;
    cvn++;
    ctu++;
  }
  //---Allocate indices --------------------------------- ------
  int dm    = (3 * nbfaces);
  int *inx  = part->AllocateXList(dm);
  //--Read faces -----------------------------------------------
  int f1,f2,f3;
	txd.azp   = 0x00;								//Tsp;
	txd.Dir   = FOLDER_ART;
	tREF			= globals->txw->Get3DTexture(txd);			//GetM3DPodTexture(txd);
	//--- TODO: Decuser  dletee part etc
	char * err = "No texture";
	if (0 == tREF)				return StopParse(p,err);
	part->SetTREF(tREF);
	if (tREF->NoData())		return StopParse(p,err);			
  int np = 0;                             // Part number
  for (int n=0; n < nbfaces; n++)
  { pgets (s, 256, p);
    nf = sscanf (s, "%d,%d,%d", &f1, &f2, &f3);
    if (3 != nf)      return StopParse(p,"FACE");
    *inx++ = f3;
    *inx++ = f2;
    *inx++ = f1;
  }
  //---Add a new part --------------------------------------------
	if (Res == MODEL_HI)	partQ.PutLast(part);
  else delete part;
  part = 0;
  return 0;
}
//----------------------------------------------------------------------
//  Get Part model
//----------------------------------------------------------------------
int CSMFparser::ReadType(char *name)
{ //---remove ending spaces ------
  char *end = name + strlen(name) - 1;
  while (*end == ' ') end--;
  end++;
  *end = 0;
  if (strncmp (name, "TRANSPL",7) == 0)   
      { Res = MODEL_LO, Tsp = 0x00; 
        return 0;
      }
  if (strncmp (name, "OPAQUEL",7) == 0)   
      { Res = MODEL_LO, Tsp = 0xFF; 
        return 0;}
  if (strncmp (name, "TRANSP",6) == 0)    
      { Res = MODEL_HI, Tsp = 0x00; 
        return 0;
      }
  if (strncmp (name, "OPAQUE",6) == 0)    
      { Res = MODEL_HI, Tsp = 0xFF; 
        return 0;}
  return 0;
}
//=============================================================================
//  BIN File decoder
//  NOTES:  All coordinates are supposed to be in feet relative to object origin
//=============================================================================
//---------------------------------------------------------------------
//  Parser for 3Dmodel
//---------------------------------------------------------------------
CBINparser::CBINparser(char t)
	:CParser(t)
{ Tsp   = 0xFF;
  trace = 0;
  nFace = 0;
  shiny = 0;
  Res   = MODEL_HI;
  xOBJ  = 0;
  nVTX  = 0;
}
//---------------------------------------------------------------------
//  Free all resources
//---------------------------------------------------------------------
CBINparser::~CBINparser()
{ if (nVTX) delete [] nVTX;
}
//-----------------------------------------------------------------------
//  return a correct S coordinate
//-----------------------------------------------------------------------
float CBINparser::GetSValue(int vs)
{ float d = float(256) * (0xFF00);
  return float(vs) / d;
}
//-----------------------------------------------------------------------
//  return a correct T coordinate
//-----------------------------------------------------------------------
float CBINparser::GetTValue(int vt)
{ float d = float(256) * (0xFF00);
  return (float(vt) / d);
}

//----------------------------------------------------------------------------
//  Decode the BIN file
//  -mod indicates weither it is a DAY/NIGHT object
//----------------------------------------------------------------------------
int CBINparser::Decode(char *fn, char t)
{ char buf[8];
  int tp = 0;
	type   = t;
  PODFILE *p = popen (&globals->pfs,fn);
	pod				 = p;
  fname			 = fn;
  if (0 == p)                     return StopParse(0, "No FILE");
//	TRACE("MODEL %s",fn);
  //----Read file type --------------------------------------
  pread (buf, sizeof(int),1, p);
  tp = *(int*)(buf);
  if (0x14 != tp)                 return StopParse(p,"TYPE");
  //---Read Header and allocate vertex table ----------------
  if (0 == ReadHeader(p))         return StopParse(p,"No HDR");

  if (0 == ReadVertex(p))         return StopParse(p,"No VTX");
  //---Read Block type and dispatch -------------------------
  while (ReadBlock(p) && (retcode == M3D_LOADED))  continue;
  if (pod) pclose (pod);
  return retcode;										//M3D_LOADED;
}

//----------------------------------------------------------------------------
//  Read Header
//----------------------------------------------------------------------------
int CBINparser::ReadHeader(PODFILE *p)
{ char buf[64];
  if (4 != pread (buf, sizeof(int),4, p))   return 0;
  hdr.scale = *(int*)(buf);
  hdr.nvert = *(int*)(buf + SIZE_INT3);
  return 1;
}
//----------------------------------------------------------------------------
//  Read Vertex
//  coordinates are supposed to be in feet? relative to object origin?
//  NOTE : Y and Z are inverted from LH to RH coordinates
//----------------------------------------------------------------------------
int CBINparser::ReadVertex(PODFILE *p)
{ int  nbv = hdr.nvert;
  char buf[32];
  nVTX     = new F3_VERTEX[nbv];
  F3_VERTEX *vtx = nVTX;
  int x,y,z;
  float scale = float(1) / 256;
  for (int k=0; k<nbv; k++)
  { if (3!= pread (buf, sizeof(int),3, p))  return 0;
    x   = *(int*)buf;
    y   = *(int*)(buf + SIZE_INT1);
    z   = *(int*)(buf + SIZE_INT2);
    vtx->VT_X = float(x) * scale;
    vtx->VT_Y = float(z) * scale;
    vtx->VT_Z = float(y) * scale;
    //---Want to save model extension here ------------
    SaveExtension(*vtx);
    //---Next component -------------------------------
    vtx++;
  }
  return 1;
}
//----------------------------------------------------------------------------
//  Read a color
//----------------------------------------------------------------------------
int CBINparser::ReadColor(PODFILE *p)
{ char buf[8];
  if (4 != pread(buf,sizeof(char),4,p))  return StopParse(p,"COLOR");
  return 1;
}
//----------------------------------------------------------------------------
//  Read a Block and dispatch to process
//----------------------------------------------------------------------------
int CBINparser::ReadBlock(PODFILE *p)
{ char buf[8];
  int tp;
  if (1 != pread (buf, sizeof(int),1, p)) return StopParse(p,"No data");
  tp = *(int*)(buf);
  switch (tp) {
    //---Normal vectors ---------------
    case 0x03:
      return ReadNormal(p);
    //---Texture name -----------------
    case 0x0D:
      return ReadTexture(p);
    //---Transparent color (number 0) -
    case 0x11:
      Tsp  = 0;
      return ReadTFaces(p);
    //---Transparent color (0) ------
    case 0x33:
      Tsp   = 0;
      return ReadTFaces(p);
    //---Opaque color (number 0) -
    case 0x18:
      Tsp   = 0xFF;
      return ReadTFaces(p);
    //---Shiny texture ----------------
    case 0x29:
      shiny = 1;
      return ReadTFaces(p);
    //---Color ------------------------
    case 0x0a:
      return ReadColor(p);
    //---End of file -----------------
    case 0x00:
			// must add last part
			if (part)	AddToModel(part);
			part	= 0;
      return 0;
  }
  return StopParse(p,"TYPE");
}
//----------------------------------------------------------------------------
//  Read Vertex
//  coordinates are supposed to be in feet? relative to object origin?
//  NOTE Y and Z are inverted for LH to RH
//----------------------------------------------------------------------------
int CBINparser::ReadNormal(PODFILE *p)
{ char *err = "NORM";
  int nbv = hdr.nvert;
  char buf[32];
  int x,y,z;
  float scale = float(1) / 65535;
  for (int k=0; k<nbv; k++)
  { if (3!= pread (buf, sizeof(int),3, p))  return StopParse(p,err);
    x   = *(int*)buf;
    y   = *(int*)(buf + SIZE_INT1);
    z   = *(int*)(buf + SIZE_INT2);
  }
  //---ByPass 2 integers ------------------------------------
  int s,t;
  if (2!= pread (buf, sizeof(int),2, p))  return StopParse(p,err);
  s     = *(int*)(buf);
  t     = *(int*)(buf + SIZE_INT1);
  part   = 0;
  return 1;
}
//----------------------------------------------------------------------------
//  Read texture name
//	We allocate a new part when texture name change
//  NOTE: Texture reference is defered until we get the opaque/transparent mode
//			of the part
//----------------------------------------------------------------------------
int CBINparser::ReadTexture(PODFILE *p)
{ char buf[64];
  char txn[PATH_MAX];
  pread (buf, sizeof(int),1, p);          // By pass unknown
  if (16 != pread (txn, sizeof(char),16, p)) return StopParse(p,"TEXT");
  txn[23] = 0;
  _strupr_s(txn,32);
	if (strcmp(txn,txname) == 0)		return 1;
	//--- Change in texture reference.  Process previous part --------------
	if (part)	AddToModel(part);
	//--- Allocate a new part ----------------------------------------------
	part	= new C3DPart(FOLDER_ART,"",0,0,0);
	part->BinRendering();
	strncpy(txname,txn,24);
  return 1;
}

//----------------------------------------------------------------------------
//  Read faces and allocate a new part
//  ignore magic number (cross produce of normal with first vertex)
//  Invert vertex direction
//  For Bin file:
//    The Vertex part is formed by each vertex value.  There is no
//    reusing of Vertex.
//  The C3Dpart  canonical format is
//    A shared vertex list    (V0,V1,...,Vn) where Vk=(x,y,z)
//    A shared normal list    (N0,N1,...,Nn) where Nk=(m,n,p)
//    A shared tex coord list (T0,T1,...,Tn) where Tk=(s,t)
//    A List of indices in above array (I0,I1,.. Ip)
//  
//  For bin file, the List of indices is (0,1,2,..,n)
//  NOTE: As we use intermediate part with texture reference, we must
//				request each time a reference because when intermediate part is
//				deleted, the reference is deallocated if no more used.
//----------------------------------------------------------------------------
int CBINparser::ReadTFaces(PODFILE *p)
{ char *err = "TFACE";
  char buf[64];

  int nx,ny,nz;
  if (1 != pread (buf, sizeof(int),1, p)) return StopParse(p,err);
  Nbv  = *(int*)(buf);
	//--- need 3 * (nbv - 2)  vertices ---------------------
	part->ExtendTNV(3 * (Nbv-2),0);						// Extend by nb vertices
  if (Nbv != 3)  return ReadQFaces(p);
  if (4 != pread (buf, sizeof(int),4, p)) return StopParse(p,err);
  nx  = *(int*)(buf);
  ny  = *(int*)(buf + SIZE_INT1);
  nz  = *(int*)(buf + SIZE_INT2);
  norm.VT_X = TC_BIN_SCALE(nx);         
  norm.VT_Y = TC_BIN_SCALE(ny);         
  norm.VT_Z = TC_BIN_SCALE(nz);
	
  part->SetTSP(Tsp);								// Set transparent mode

  //-----Build the faces -(invert Y and Z normals) ------
  for (int k=0;k < Nbv; k++)
  { if (3 != pread (buf, sizeof(int),3, p)) return StopParse(p,err);
    int vx  = *(int*)(buf);									// Index in Vertice
    int sv  = *(int*)(buf + SIZE_INT1);			
    int tv  = *(int*)(buf + SIZE_INT2);	
    ctex.VT_S = GetSValue(sv);
    ctex.VT_T = GetTValue(tv);
		part->Push(nVTX[vx],norm,ctex);
  }
  //---count a face ----------------
	part->AddFace(1);
  return 1;
}

//---------------------------------------------------------------------------------
//  Read QUAD faces:  Separate in 2 triangle parts
//  Vertex list 0,1,2,3 is rewriten as 0,1,2,0,2,3
//---------------------------------------------------------------------------------
int CBINparser::ReadQFaces(PODFILE *p)
{ char *err = "QFACE";
  char buf[64];
  int nx,ny,nz;
  F3_VERTEX vref[4];
  F3_VERTEX nref[4];
  F2_COORD  tref[4];
  if (Nbv != 4)                           return StopParse(p,err);
  if (4 != pread (buf, sizeof(int),4, p)) return StopParse(p,err);
  nx  = *(int*)(buf);
  ny  = *(int*)(buf + SIZE_INT1);
  nz  = *(int*)(buf + SIZE_INT2);
  norm.VT_X = TC_BIN_SCALE(nx);         //float(nx) * norme;
  norm.VT_Y = TC_BIN_SCALE(ny);         //float(ny) * norme;
  norm.VT_Z = TC_BIN_SCALE(nz);         //float(nz) * norme;
  //-----Read the faces -(invert Y and Z normals) ------
  for (int k=0;k < 4; k++)
  { if (3 != pread (buf, sizeof(int),3, p)) return StopParse(p,err);
    int vx  = *(int*)(buf);
    int sv  = *(int*)(buf + SIZE_INT1);
    int tv  = *(int*)(buf + SIZE_INT2);
		ctex.VT_S = GetSValue(sv);
		ctex.VT_T = GetTValue(tv);
    if (vx >= hdr.nvert)										return StopParse(p,err);
		//--- save faces parameters -----------------------------
    vref[k]       = nVTX[vx];
    nref[k]				= norm;
    tref[k]       = ctex;
  }
  //----Build First triangle -------------------------------
	part->Push(vref[0],nref[0],tref[0]);
	part->Push(vref[1],nref[1],tref[1]);
	part->Push(vref[2],nref[2],tref[2]);
	//--- Build second triangle ------------------------------
	part->Push(vref[0],nref[0],tref[0]);
	part->Push(vref[2],nref[2],tref[2]);
	part->Push(vref[3],nref[3],tref[3]);
	//---------------------------------------------------------
  part->SetTSP(Tsp);
	part->AddFace(2);
  return 2;
}
//------------------------------------------------------------------------------------
//  Add Part to model
//------------------------------------------------------------------------------------
void CBINparser::AddToModel(C3DPart *prt)
{	prt->SetTexture(Tsp,txname);
	prt->AllocateIND();
	partQ.PutLast(prt);
  return;
}

//==================================================================================
//  RLP parser
//==================================================================================
CRLParser::CRLParser(CAirportMgr *ap, char *fn)
{ char fkey[PATH_MAX];
  apm = ap;
  //---TRY to locate profile ---------------------------------
  sprintf_s(fkey,FNAM_MAX,"%s.RLP",fn);
  Decode(fkey);
}
//----------------------------------------------------------------------------------
//  Decode the file if it exists
//----------------------------------------------------------------------------------
void CRLParser::Decode(char *fk)
{ SStream s(this,fk);
  return;
}
//----------------------------------------------------------------------------------
//  Read Light Profile
//----------------------------------------------------------------------------------
int CRLParser::Read(SStream *st,Tag tag)
{ Tag pm;
  switch(tag) {
  //---Runway end  ----------------------------
    case 'rwId':
        memset(&prof,0,sizeof(RWY_EPF));
        ReadString(prof.end,4,st);
        ReadFrom(this,st);
        return TAG_READ;
    //---APPROACH LIGHTS ----------------------
    case 'aprL':
        ReadTag(&pm,st);
        if (pm == 'none') prof.aprL = TC_APR_NONE;
        if (pm == 'REIL') prof.aprL = TC_APR_REIL;
        if (pm == 'ODAL') prof.aprL = TC_APR_ODAL;
        if (pm == 'ALSR') prof.aprL = TC_APR_ALSR;
        if (pm == 'ALF1') prof.aprL = TC_APR_ALF1;
        if (pm == 'ALF2') prof.aprL = TC_APR_ALF2;
        return TAG_READ;
    //---THRESHOLD BAR ------------------------
    case 'Tbar':
        ReadTag(&pm,st);
        if (pm == 'Rend') prof.Tbar = TC_TBAR_RED;
        if (pm == 'Gend') prof.Tbar = TC_TBAR_GRN;
        if (pm == '2end') prof.Tbar = TC_TBAR_RGN;
        if (pm == 'inop') prof.Tbar = TC_TBAR_NOP;
        if (pm == 'none') prof.Tbar = TC_TBAR_SUP;
        return TAG_READ;
    // ---WING BAR ---------------------------
    case 'Wbar':
        ReadTag(&pm,st);
        if (pm == 'yes_')  prof.Wbar = TC_OPTP_YES;
        if (pm == 'none')  prof.Wbar = TC_OPTP_SUP;
        return TAG_READ;
    //----TOUCH DOWN ZONE ---------------------
    case 'tdzL':
        ReadTag(&pm,st);
        if (pm == 'yes_')  prof.tdzL = TC_OPTP_YES;
        if (pm == 'none')  prof.tdzL = TC_OPTP_SUP;
        return TAG_READ;
    //----CENTER SEGMENT ---------------------
    case 'ctrL':
        ReadTag(&pm,st);
        if (pm == 'none') prof.ctrL = 1;
        if (pm == 'mono') prof.ctrL = 2;
        if (pm == 'CATx') prof.ctrL = 3;
        return TAG_READ;
    //--- EDGE SEGMENT -----------------------
    case 'edgL':
        ReadTag(&pm,st);
        if (pm == 'none') prof.edgL = 1;
        if (pm == 'mono') prof.edgL = 2;
        if (pm == 'CATx') prof.edgL = 3;
        return TAG_READ;
    //--- PAPI SYSTEM -----------------------
    case 'PAPI':
        ReadTag(&pm,st);
        if (pm == 'none') prof.papL = TC_PAPI_NL;
        if (pm == '4lit') prof.papL = TC_PAPI_4L;
        if (pm == '2lit') prof.papL = TC_PAPI_2L;
        return TAG_READ;
    default:
      break;
  }
  return TAG_IGNORED;
}
//-----------------------------------------------------------------------
//  Parameters for a runway end are read.  Add this to the global table
//-----------------------------------------------------------------------
void CRLParser::ReadFinished()
{ if (*prof.end) apm->AddProfile(prof);
  prof.end[0] = 0;
  return;
}
//==================================================================================
//  METAR parser
//==================================================================================
METARparser::METARparser(FILE *f,METAR_INFO *inf,char *fn)
{ filr = f;
  info = inf;
  strncpy(info->iden,fn,4);
  info->iden[4] = 0;
  info->code    = 0;
  Parse();
}
//-----------------------------------------------------------------------
//  Warning
//-----------------------------------------------------------------------
int METARparser::Warn(char *msg)
{ WARNINGLOG("%s %s",info->iden,msg);
  info->code = 1;
  fclose(filr);
  return 0;
}
//-----------------------------------------------------------------------
//  Parse the file
//-----------------------------------------------------------------------
int METARparser::Parse()
{ int  nf;
  int  en;
  char mask[PATH_MAX];
	int  d = FNAM_MAX;
  //----Parse the METAR DATA --------------------------------
  sprintf_s(mask,d,"%s %%dZ",info->iden);
  //----Clear all infos -------------------------------------
  nf = fscanf(filr,mask,&en);
  if (1 != nf)  return Warn("PB ident");   // Ident and date
  info->wDR1  = 0;
  info->wDR2  = 0;
  info->wASP  = 0;
  info->wGSP  = 0;
  info->wSTR  = 0;
  info->wQAL  = 0;
  info->Layer = 0;
  info->Cover = 0;
  info->Ceil  = 0;
  info->VertV = 0;
  info->vMIL  = 0;
  info->Temp  = 0;
  info->DewP  = 0;
  //---Bypass COR / AUTO ------------------------------------
  fscanf(filr," COR");
  fscanf(filr," AUTO");
  ParseWind(filr);
  ParseWdP2(filr);
  (*info->iden == 'K')?(ParseUSvis(filr)):(ParseEUvis(filr));
  //---Decode body ----------------------------
  ParseBody(filr);
  fclose(filr);
  return 0;
}
//------------------------------------------------------------------------------
//  Skip a filed
//------------------------------------------------------------------------------
int METARparser::SkipField(FILE *f)
{ char fd[32];
  int  rp = fscanf(f,"%32s ",fd);
  return (rp == EOF)?(0):(1);
}
//------------------------------------------------------------------------------
//  Parse the METAR BODY
//------------------------------------------------------------------------------
int METARparser::ParseBody(FILE *f)
{ int go = 1;
  int no = 0;
  while (go)
  { no = 0;
    while (ParseRWvis(f))   continue;
    while (ParseCover(f))   continue;
    while (ParseWeather(f)) continue;
    no  += ParseTemp(f);
    no  += ParseBARO(f);
    if (2 == no)            return 0;
    if (SkipField(f))       continue;
  }
  return 0;
}
//------------------------------------------------------------------------------
//  Parse wind part 1
//------------------------------------------------------------------------------
int METARparser::ParseWind(FILE *f)
{ int *w1 = &info->wDR1;
  int *w2 = &info->wASP;
  int *w3 = &info->wGSP;
  long pos = ftell(f);
  if (3 == fscanf(f," %3d%dG%dKT",w1,w2,w3))
  { return 1; }                     // All data are set
  fseek(f,pos,0);
  if (2 == fscanf(f," %3d%dKT",w1,w2))
  { info->wGSP = *w2;                 // Set gust to average
    return 1; }
  fseek(f,pos,0);
  if (1 == fscanf(f," VRB%dKT",w2))
  { info->wGSP = *w2;                 // Set gust to average
    return 1; }
  fseek(f,pos,0);
  return 0;
}
//------------------------------------------------------------------------------
//  Parse wind part 2
//------------------------------------------------------------------------------
int METARparser::ParseWdP2(FILE *f)
{ int w1;
  int w2;
  long pos = ftell(f);
  if (2 != fscanf(f," %3dV%3d",&w1,&w2)) { fseek(f,pos,0); return 0;  }     
  info->wDR1 = w1;
  info->wDR2 = w2;
  return 0;
}
//------------------------------------------------------------------------------
//  Parse US visibility
//------------------------------------------------------------------------------
int METARparser::ParseUSvis(FILE *f)
{ int p1,p2,p3;
  long pos = ftell(f);
  if (3 == fscanf(f," %d %d/%dSM ",&p1,&p2,&p3))
  { info->vMIL = double(p1) + (double(p2)/p3);
    return 1;}
  fseek(f,pos,0);
  if (2 == fscanf(f," %d/%dSM ",&p1,&p2))
  { info->vMIL = double(p1)/p2;  
    return 1;}
  fseek(f,pos,0);
  if (1 == fscanf(f," %dSM ", &p1))             
  { info->vMIL = double(p1);     
    return 1;}
  fseek(f,pos,0);
  return 0;
}
//------------------------------------------------------------------------------
//  Parse EU visibility
//  Bypass NDV
//------------------------------------------------------------------------------
int METARparser::ParseEUvis(FILE *f)
{ int vm = 0;
  long pos = ftell(f);
  if (1 == fscanf(f," %4d ",&vm))
  { info->vMIL = FN_MILE_FROM_METER(vm);
    fscanf(f,"NDV ");                 
    return 1;}
  fseek(f,pos,0);
  return 0;
}
//------------------------------------------------------------------------------
//  Parse Runways RVR
//------------------------------------------------------------------------------
int METARparser::ParseRWvis(FILE *f)
{ int rw;
  int lg;
  long pos = ftell(f);
  if (1 != fscanf(f," R%2d", &rw))  {fseek(f,pos,0); return 0;}
  //---Bypass runway designation -----------------------------
  if (1 == fscanf(f,"/%4d", &lg))     return ParseRWend(f);
  if (1 == fscanf(f,"R/%4d",&lg))     return ParseRWend(f);
  if (1 == fscanf(f,"L/%4d",&lg))     return ParseRWend(f);
  if (1 == fscanf(f,"C/%4d",&lg))     return ParseRWend(f);
  fseek(f,pos,0);
  return 0;
}
//------------------------------------------------------------------------------
//  Parse Runways RVR part 2
//------------------------------------------------------------------------------
int METARparser::ParseRWend(FILE *f)
{ int lg;
  fscanf(f,"V%4d",&lg);
  fscanf(f,"FT ");
  return 1;
}
//------------------------------------------------------------------------------
//  Parse weather qualifier
//------------------------------------------------------------------------------
int METARparser::ParseWeather(FILE *f)
{ long pos = ftell(f);
  if (1 == fscanf(f,"+%2s",fld) && (WeatherP1(f,'+')))  return 1;
  fseek(f,pos,0);
  if (1 == fscanf(f,"-%2s",fld) && (WeatherP1(f,'-')))  return 1;
  fseek(f,pos,0);
  if (1 == fscanf(f,"%2s", fld) && (WeatherP1(f,0)))    return 1;
  fseek(f,pos,0);
  if (1 == WeatherP2(f))                                return 1;
  fseek(f,pos,0);
  return 0;
}
//------------------------------------------------------------------------------
//  Parse weather qualifier
//------------------------------------------------------------------------------
int METARparser::WeatherP1(FILE *f,char c)
{ Tag q = 0;
  q |= (fld[0] << 8);
  q |= (fld[1]);
  for (int k=0; qalMETAR[k]!= 0; k++)
  { if (qalMETAR[k] != q)  continue;
    info->wSTR = c;
    info->wQAL = q;
    return WeatherP2(f);
  }
  //--- Not a known weather qualifier --------------
  return 0;
}
//------------------------------------------------------------------------------
//  Parse weather type
//------------------------------------------------------------------------------
int METARparser::WeatherP2(FILE *f)
{ fld[0] = 0;
  fscanf(f,"%2s ",fld);
  Tag q  = 0 | (fld[0] << 8) | (fld[1]);
  for (int k=0; wthMETAR[k]!= 0; k++)
  { if (wthMETAR[k] != q)  continue;
    info->wTYP = q;
    return 1;
  }
  //--- Not a known weather type --------------
  return 0;
}
//------------------------------------------------------------------------------
//  Parse cloud cover
//------------------------------------------------------------------------------
int METARparser::ParseCover(FILE *f)
{ int  vv;
  long pos = ftell(f);
  if (CoverP1(f))                   return 1;
  //--- Check CAVOK -----------------------------------
  fseek(f,pos,0);
  fscanf(f," %10s",fld);
  if (strncmp(fld,"CAVOK",5)     == 0)  return 1;
  if (strncmp(fld,"//////CB",8)  == 0)  return 1;
  if (strncmp(fld,"//////TCU",9) == 0)  return 1;
  //--- Check Vertical visibility --------------------
  fseek(f,pos,0);
  if (1 == fscanf(f,"VV%3d",&vv))  {info->VertV = (vv * 100); return 1;}
  fseek(f,pos,0);
  return 0;
}
//------------------------------------------------------------------------------
//  Parse cloud cover part 1
//------------------------------------------------------------------------------
int METARparser::CoverP1(FILE *f)
{ Tag q = 0;
  fscanf(f," %3s",fld);
  q  = 0 | (fld[0] << 16) | (fld[1] << 8) | (fld[2]);
  //----Check any cover ------------------------------
  for (int k=0; covMETAR[k] != 0; k++)
  { if (covMETAR[k] != q) continue;
    info->Layer = layMETAR[k];
    info->Cover = q;
    return (info->Layer)?(CoverP2(f)):(1);
  }
  //---- Not a part 1 --------------------------------
  return 0;
}
//------------------------------------------------------------------------------
//  Parse cloud cover part 1
//------------------------------------------------------------------------------
int METARparser::CoverP2(FILE *f)
{ int cl = 0;
  if (1 != fscanf(f,"%3d",&cl)) return 0;
  info->Ceil  = (cl * 100);
  int k  = 0;
  k |= fscanf(f,"CB ");              // Bypass CB
  k |= fscanf(f,"TCU ");             // Bypass TCU
  if (k) info->Layer = 6;
  fscanf(f,"/// ");                   // Bypass ////
  return 1;
}
//------------------------------------------------------------------------------
//  Parse Temperature
//------------------------------------------------------------------------------
int METARparser::ParseTemp(FILE *f)
{ long pos = ftell(f);
  int tp;
  int dw;
  if (2 == fscanf(f,"%d/%d ",&tp,&dw))
  { info->Temp  = float(tp);
    info->DewP  = float(dw);
    return 1;}
  //---Not temperature --------------------
  fseek(f,pos,0);
  return 0;
}
//------------------------------------------------------------------------------
//  Parse baro presure
//------------------------------------------------------------------------------
int METARparser::ParseBARO(FILE *f)
{ if (ParseAlti(f)) return 1;
  if (ParseQNH(f))  return 1;
  return 0;
}
//------------------------------------------------------------------------------
//  Parse Altimeter in inch of mercury
//------------------------------------------------------------------------------
int METARparser::ParseAlti(FILE *f)
{ long pos = ftell(f);
  int en;
  int pn;
  if (2 == fscanf(f,"A%2d%2d ",&en,&pn))
  { float inhg = double (en) + double (pn * 0.01);
    info->Baro = INHG_TO_PAS * inhg * 0.01;        // In hPa
    return 1;
  }
  //---Not Altimeter --------------------
  fseek(f,pos,0);
  return 0;
}
//------------------------------------------------------------------------------
//  Parse QNH in milibar
//------------------------------------------------------------------------------
int METARparser::ParseQNH(FILE *f)
{ long pos = ftell(f);
  int hp;
  if (1 == fscanf(f,"Q%4d ",&hp))
  { info->Baro = hp;           // In hPa = 1 mbar
    return 1;
  }
  //---Not Altimeter --------------------
  fseek(f,pos,0);
  return 0;
}
//===================================================================================
//  Bt Parser for elevations
//===================================================================================
CBtParser::CBtParser(char *fn)
{ fname = fn;
  fp  = fopen(fn,"rb");
  if (0 == fp)  return;
  fsiz  = _filelength(fp->_file);
  //----------------------------------------------------------------
  tDim  = 5;
  step  = 256;
  //---Open the header ---------------------------------------------
  fread(&Head.ident, 10, 1, fp);
  fread(&Head.ncol , sizeof(int),     1, fp);   // Number of columns
  fread(&Head.nrow , sizeof(int),     1, fp);   // Number of rows
  fread(&Head.dsiz , sizeof(short),   1, fp);   // Data size
  fread(&Head.dtyp , sizeof(short),   1, fp);   // Data type
  fread(&Head.unit , sizeof(short),   1, fp);   // Unit
  fread(&Head.zUTM , sizeof(short),   1, fp);   // UTM cell
  fread(&Head.datum, sizeof(short),   1, fp);   // Datum
  fread(&Head.xlef , sizeof(double),  1, fp);   // Left coordinate
  fread(&Head.xrig , sizeof(double),  1, fp);   // Right
  fread(&Head.xbot , sizeof(double),  1, fp);   // Bottom
  fread(&Head.xtop , sizeof(double),  1, fp);   // Top
  fread(&Head.proj , sizeof(short) ,  1, fp);   // Projection
  fread(&Head.scale, sizeof(float) ,  1, fp);   // Scale
  if (Head.scale == 0)  Head.scale = 1;
  //---Check expected parameters --------------------------------
  if (Head.dtyp != 1) gtfo("Elevations must be floats. File %s",      fname);
  if (Head.unit != 0) gtfo("Grid units must be degre. File %s",       fname);
  if (Head.scale!= 1) gtfo("Elevation must be in meters. File %s",    fname);
  //----Read elevation (column first -----------------------------
  int nbf = Head.ncol * Head.nrow;
  data    = new float[nbf];
  float  *dst = data;
  fseek(fp,long(256),SEEK_SET);
  int    nbe = 0;
  for (int r=0; r < Head.nrow; r++)
  { for (int c=0; c < Head.ncol; c++)
    { float f = 0;
      float e = 0;
      int a = fread(&f,sizeof(float),1,fp);
      if (0 == a) gtfo("Read error. File %s",fname);
      e = f * TC_FEET_PER_METER;
     *dst++ = e;
      nbe++;
    }
  }
  fclose(fp);
  //---Normalize the longitudes  -----------------------------------
  double lw = Head.xlef;
  if (lw < 0) lw += 360;
  double le = Head.xrig;
  if (le < 0) le += 360;
  //---Compute limits in arcsec (3600 arcsec per deg ---------------
  lefS = 3600 * lw;
  rigS = 3600 * le;
  topS = 3600 * Head.xtop;
  botS = 3600 * Head.xbot;
  //-----Compute grid spacing in arcsec -------------
  horz  = LongitudeDifference(rigS,lefS) / (Head.ncol - 1);
  vert  = (topS - botS) / (Head.nrow - 1);
  //-----Compute details tiles keys  ----------------
  ComputeTileLimits();
}
//-------------------------------------------------------------------
//  Destructor
//-------------------------------------------------------------------
CBtParser::~CBtParser()
{ delete [] data;
}
//------------------------------------------------------------------
//  Compute base detail tile
//  There are 16384 detail tiles in horizontal direction for 360°
//------------------------------------------------------------------
void CBtParser::ComputeTileLimits()
{ double dpa = double(16384) / TC_FULL_WRD_ARCS;         // Detail tile per arcsec
  //---Get the lefmost enclosed detail tile ----
  mREG.xmin = int (double(lefS) * dpa) + 1;
  //---Get the righmost included detail tile ---
  mREG.xmax = int (double(rigS) * dpa) - 1;
  //---Get the bottomost enclosed DT -----------
  mREG.zmin = GetTileFromLatitude(botS) + 1;
  //---Get the topmost excluded DT -------------
  mREG.zmax = GetTileFromLatitude(topS) - 1;
  //---TRACE AREA OF EXPORT -------------------
  qReg.qx0 = (mREG.xmin >> TC_BY32);
  qReg.qz0 = (mREG.zmin >> TC_BY32);
  qReg.tx0 = (mREG.xmin & TC_032MODULO);
  qReg.tz0 = (mREG.zmin & TC_032MODULO);
  TRACE("ELEVATION AREA start  Q(%03d-%03d)  D(%02d-%02d)",qReg.qx0,qReg.qz0,qReg.tx0,qReg.tz0);
  qReg.qx1 = (mREG.xmax >> TC_BY32);
  qReg.qz1 = (mREG.zmax >> TC_BY32);
  qReg.tx1 = (mREG.xmax & TC_032MODULO);
  qReg.tz1 = (mREG.zmax & TC_032MODULO);
  TRACE("ELEVATION AREA end at Q(%03d-%03d)  D(%02d-%02d)",qReg.qx1,qReg.qz1,qReg.tx1,qReg.tz1);
  //----Check area size ------------------------
  if (qReg.qx1 < qReg.qx0)  gtfo("X-End tile < X-start tile");
  if (qReg.qz1 < qReg.qz0)  gtfo("Z-End tile < Z-start tile");
  //--------------------------------------------
  qReg.nqx = QgtDifference(qReg.qx1,qReg.qx0);
  qReg.nqz = QgtDifference(qReg.qz1,qReg.qz0);
  qReg.qtx = qReg.qx0;
  qReg.qtz = qReg.qz0;
  qReg.dtx = qReg.tx0;
  qReg.dtz = qReg.tz0;
  //---Compute last tile key ------------------
  qReg.ftx = (qReg.nqx)?(31):(qReg.tx1);
  qReg.ftz = (qReg.nqz)?(31):(qReg.tz1);
  nxt = true;
  return;
}
//------------------------------------------------------------------
//    Get the current QGT key
//------------------------------------------------------------------
bool CBtParser::GetQgtKey(U_INT &xk,U_INT &zk)
{ xk  = qReg.qtx;
  zk  = qReg.qtz;
  return (nxt);
}
//------------------------------------------------------------------
//    Get the next QGT key
//------------------------------------------------------------------
bool CBtParser::NextQgtKey()
{ qReg.qtx++;
  qReg.dtx  = 0;
  if (qReg.qtx == qReg.qx1) qReg.ftx = qReg.tx1;
  if (qReg.qtx <= qReg.qx1) return true;
  qReg.dtz = 0;
  qReg.qtx = qReg.qx0;
  qReg.dtx = qReg.tx0;
  qReg.ftx = 31;
  qReg.ftz = 31;
  qReg.qtz++;
  if (qReg.qtz == qReg.qz1) qReg.ftz = qReg.tz1;
  if (qReg.qtz <= qReg.qz1) return true;
  nxt = false;
  return false;
}
//------------------------------------------------------------------
//  Compute Elevation for vertex identified by indices
//------------------------------------------------------------------
bool CBtParser::VertexElevation(U_INT vx,U_INT vz)
{ GetVertexCoordinates(vx,vz,dot);
  //--- Compute SW elevation indice --------
  double lgx = dot.x - lefS;
  int    col = int (lgx / horz);
  double lgy = dot.y - botS;
  int    row = int (lgy / vert);
  if (col <          0) gtfo("Col error. File %s",fname);
  if (row <          0) gtfo("Row error. File %s",fname);
  if (col >= Head.ncol) gtfo("Row error. File %s",fname);
  if (row >= Head.nrow) gtfo("Col error. File %s",fname);
  //----Fill surrounding corners ------------------------
  SW.x  = lefS + (col * horz);
  SW.y  = botS + (row * vert);
  SW.z  = GetElevation(col,row);
  //--- NorthWest corner --------------------------------
  NW    = SW;
  NW.y += vert;
  NW.z  = GetElevation(col,row+1);
  //---SouthEast corner ---------------------------------
  SE    = SW;
  SE.x += horz;
  SW.z  = GetElevation(col+1,row);
  //--- NE corner ---------------------------------------
  NE    = SW;
  NE.x += horz;
  NE.y += vert;
  NE.z  = GetElevation(col+1, row+1);
  //--- Test for elevation in lower triangle ------------
  if (LowerTriangle())  return true;
  if (UpperTriangle())  return true;
  gtfo("Grid error. file %s",fname);
  return false;
}
//------------------------------------------------------------------
//  Get elevation at Col,Row
//  NOTE:  Elevations in bt file are stored column first
//    col 0 cccccccccccc
//    col 1 cccccccccccc
//    etc
//------------------------------------------------------------------
float CBtParser::GetElevation(int c,int r)
{ float  *item = data + (c * Head.nrow) + r;
  return *item;
}
//------------------------------------------------------------------
//  Check for point in lower triangle
//  Compute relative coordinates for points:
//  P (the tile vertex)
//  A the SW corner
//  B the SE corner
//  C the NW corner
//------------------------------------------------------------------
bool CBtParser::LowerTriangle()
{ double dx = LongitudeDifference(dot.x,SW.x);
  CVector n;
  CVector p(   dx,(dot.y - SW.y),0);
  CVector b(+horz,     0, SE.z);
  CVector c(    0, +vert, NW.z);
  if (!PointInTriangle(p,SW,b,c,n)) return false;
  dot.z = p.z;
  return true;
}
//------------------------------------------------------------------
//  Check for point in upper triangle
//  Compute relative coordinates for all points
//  P (the tile vertex)
//  A the NE corner
//  B the NW corner
//  C the SE corner
//------------------------------------------------------------------
bool CBtParser::UpperTriangle()
{ double dx = LongitudeDifference(dot.x,NE.x);
  CVector n;
  CVector p(   dx,(dot.y - NE.y),0);
  CVector b(-horz,      0, NW.z);
  CVector c(    0, - vert, SE.z);
  if (!PointInTriangle(p,NE,b,c,n)) return false;
  dot.z = p.z;
  return true;
}
//------------------------------------------------------------------
//  return next vertex index
//------------------------------------------------------------------
U_INT CBtParser::NextVertexKey(U_INT vk)
{ U_INT nk = vk + step;
  return (nk & TC_ABSMODULO);
}
//------------------------------------------------------------------
//  return next Tile index
//------------------------------------------------------------------
U_INT CBtParser::NextTileKey(U_INT vk)
{ U_INT nk = vk + TC_TILEINC;
  return (nk & TC_ABSMODULO);
}
//------------------------------------------------------------------
//  Add to Tile index
//------------------------------------------------------------------
U_INT CBtParser::IncTileKey(U_INT vk,U_INT inc)
{ U_INT nk = vk + inc;
  return (nk & TC_TILE_MODULO);
}
//------------------------------------------------------------------
//  Process current Tile
//------------------------------------------------------------------
int CBtParser::ProcessTile(U_INT tx,U_INT tz)
{ int  count = 0;
  U_INT   qx = qReg.qtx;
  U_INT   qz = qReg.qtz;
  TRACE("....TILE Q(%03d-%03d) D(%02d-%02d)",qx,qz,tx,tz);
  U_INT   vz = (qz << (TC_BY32 + TC_BY1024)) + (tz << TC_BY1024);
  U_INT   vx = (qx << (TC_BY32 + TC_BY1024)) + (tx << TC_BY1024);
  //----Compute tile elevations --------------------------------
  U_INT  fz  = NextTileKey(vz);
  U_INT  fx  = NextTileKey(vx);
  for   (U_INT kz = vz; kz <= fz; kz = NextVertexKey(kz))
  { for (U_INT kx = vx; kx <= fx; kx = NextVertexKey(kx))
    { VertexElevation(kx,kz);
     *elPT++ = int(dot.z);
      count++;
    }
  }
  return count;
}
//------------------------------------------------------------------
//  Get Region elevation
//------------------------------------------------------------------
bool CBtParser::GetRegionElevation(REGION_REC &reg)
{ int count = 0;
  reg.qtx = qReg.qtx;
  reg.qtz = qReg.qtz;
  reg.dtx = qReg.dtx;
  reg.dtz = qReg.dtz;
  reg.sub = tDim;
  reg.lgx = qReg.ftx - qReg.dtx + 1;
  reg.lgz = qReg.ftz - qReg.dtz + 1;
  reg.nbv = reg.lgx * reg.lgz * tDim * tDim;
  int *mat = new int[reg.nbv];
  reg.data = mat;
  elPT     = mat;
  for   (U_INT tz = qReg.dtz; tz <= qReg.ftz; tz++)
  { for (U_INT tx = qReg.dtx; tx <= qReg.ftx; tx++)
    {count += ProcessTile(tx,tz);
    }
  }
  elPT  = 0;
  return true;
}
//==================================================================================
//  OBJ format parser
//	NOTE: We add a dummy vertex to each vector list because reference to vertices 
//	Start from 1.  They wyill be deleted with the list
//==================================================================================
COBJparser::COBJparser(char t)
	:CParser(t)
{	trfm = 0;
	GN_VTAB *V = new GN_VTAB;
	vpos.push_back(V);			// Add dummy vector 0
	GN_VTAB *T = new GN_VTAB;
	vtex.push_back(T);			// Add dummy texture
	GN_VTAB *N = new GN_VTAB;
	vnor.push_back(N);			// Add dummy normal
	E	= 1;
}
//------------------------------------------------------------------------------
//	Free all resources
//------------------------------------------------------------------------------
COBJparser::~COBJparser()
{	Free(1);
}
//------------------------------------------------------------------------------
//	Free all resources
//------------------------------------------------------------------------------
void COBJparser::Free(char opt)
{	for (U_INT k=0; k < vpos.size(); k++)			delete vpos[k];
	for (U_INT k=0; k < vtex.size(); k++)			delete vtex[k];
	for (U_INT k=0; k < vnor.size(); k++)			delete vnor[k];
	vpos.clear();
	vtex.clear();
	vnor.clear();
	if (0 == opt)		return;
	//--- Free material queues -----------------------------
	for (U_INT k=0; k < matQ.size(); k++)			delete matQ[k];
	matQ.clear();
	return;
}

//------------------------------------------------------------------------------
//	Set directory 
//------------------------------------------------------------------------------
void COBJparser::SetDirectory(char *dir)
{	strncpy(txname,dir,TC_TEXTURE_NAME_DIM);
	return;
}
//------------------------------------------------------------------------------
//	Set directory 
//------------------------------------------------------------------------------
void COBJparser::SetTransform(CVector t,double c,double s,double e)
{	trfm	= 1;
	T			= t;
	S			= s;
	C			= c;
	E			= e;
	return;
}
//------------------------------------------------------------------------------
//	Decode the file
//------------------------------------------------------------------------------
int COBJparser::Decode(char *fn,char t)
{ type				= t;
	fname       = fn;
	strcat(txname,"/");
	strcat(txname,fn);
  PODFILE *p	= popen (&globals->pfs,txname);
	pod					= p;
  if (0 == p)							return StopParse(0, "No FILE ");
	//------------------------------------------------------------
	char s[256];
  //--- Object type, this should be "WaveFront"
  pgets (s, 256, p);
	//--- Bypass name ----------------------------------------------
	GetStatement(s);
	//--- Check if material is used --------------------------------
	while (GetStatement(s))
	{	if (*s == '#')										continue;
		if (ParseName(s))									continue;
		if (ParseLibrary(s))							continue;
		if (ParseMaterial(s))							continue;
		if (Parse3Vertex(s))							continue;
		if (Parse3TCoord(s))							continue;
		if (Parse3Normes(s))							continue;
		if (Parse3NFaces(s))							continue;
		if (Parse4Faces(s))								continue;
		if (Parse3Faces(s))								continue;
		if (*s == 'g')										continue;
	}
	//--- Close file  ---------------------------
	if (pod)  pclose(pod);
	return M3D_LOADED;
}
//------------------------------------------------------------------------------
//	Check for Library
//------------------------------------------------------------------------------
bool COBJparser::ParseLibrary(char *s)
{ char lib[128];
	int nf  = sscanf(s,"mtllib %128s ",lib);
	return (nf == 1);
}
//------------------------------------------------------------------------------
//	Search for material or create one
//------------------------------------------------------------------------------
OBJ_MATERIAL *COBJparser::GetMaterial(char *mn)
{	OBJ_MATERIAL *M = 0;
	for (U_INT k=0; k<matQ.size(); k++)
	{	M = matQ[k];
		if (strcmp(mn, M->name) == 0)	return M;
	}
	//--- Create a new entry ----------------------
	M	= new OBJ_MATERIAL();
	M->name	= DupplicateString(mn,64);
	matQ.push_back(M);
	return M;
}
//------------------------------------------------------------------------------
//	Check for material
//------------------------------------------------------------------------------
bool COBJparser::ParseMaterial(char *s)
{ int nf  = sscanf(s,"usemtl %128s ",txd.name);
	if (nf != 1)		return false;
	//--- Locate or create a new material ------------------
	mat	= GetMaterial(txd.name);
	return true;
}
//------------------------------------------------------------------------------
//	Check for name
//------------------------------------------------------------------------------
bool COBJparser::ParseName(char *s)
{ char txt[128];
	int nf  = sscanf(s,"g %128s ",txt);
	return (nf == 1);
}

//------------------------------------------------------------------------------
//	Check for vertex
//------------------------------------------------------------------------------
bool COBJparser::Parse3Vertex(char *s)
{	GN_VTAB v;
  float   w;
	int nf	= sscanf(s,"v %f %f %f", &v.VT_X, &v.VT_Y, &v.VT_Z, &w);
	if (nf < 3)		return false;
	GN_VTAB *vp		= new GN_VTAB();
	vp->InvertHD1(v);
	SaveExtension(*vp);
	vpos.push_back(vp);					// Add to list
	return true;
}
//------------------------------------------------------------------------------
//	Check for Texture coordinates
//------------------------------------------------------------------------------
bool COBJparser::Parse3TCoord(char *s)
{	GN_VTAB v;
  float   w;
	int nf	= sscanf(s,"vt %f %f %f", &v.VT_S, &v.VT_T, &w);
	if (nf < 2)		return false;
	GN_VTAB *vt		= new GN_VTAB();
	vt->Copy(v);
	double T = vt->VT_T;
	vtex.push_back(vt);					// Add to list
	return true;
}
//------------------------------------------------------------------------------
//	Check for Texture coordinates
//------------------------------------------------------------------------------
bool COBJparser::Parse3Normes(char *s)
{	GN_VTAB v;
	int nf	= sscanf(s,"vn %f %f %f", &v.VN_X, &v.VN_Y, &v.VN_Z);
	if (3 != nf)		return false;
	GN_VTAB *vp		= new GN_VTAB();
	vp->Copy(v);
	vnor.push_back(vp);					// Add to list
	return true;
}
//------------------------------------------------------------------------------
//	Build a triangle with vertex and texture only
//------------------------------------------------------------------------------
bool COBJparser::BuildTriangleVertex(int dst, U_INT vt, U_INT nt)
{	GN_VTAB *tab = tri.vtx + dst;
	if (vt >= vpos.size())	 return false;
	*tab  = *vpos[vt];
	if (nt >= vtex.size())	 return false;
	tab->VT_S = vtex[nt]->VT_S;
	tab->VT_T = vtex[nt]->VT_T;
	tab->VN_X = 0.5;
	tab->VN_Y = 0.5;
	tab->VN_Z	= 0.5;
	return true;
}
//------------------------------------------------------------------------------
//	Build a triangle with vertex and texture only
//------------------------------------------------------------------------------
bool COBJparser::BuildTriangleVertex(int dst, U_INT vt, U_INT nt,U_INT nm)
{	GN_VTAB *tab = tri.vtx + dst;
	if (vt >= vpos.size())		return false;
	*tab      = *vpos[vt];
	if (nt >= vtex.size())		return false;
	tab->VT_S = vtex[nt]->VT_S;
	tab->VT_T = vtex[nt]->VT_T;
	if (nm >= vnor.size())		return false;
	tab->VN_X = vnor[nm]->VN_X;
	tab->VN_Y = vnor[nm]->VN_Y;
	tab->VN_Z	= vnor[nm]->VN_Z;
	return true;
}

//------------------------------------------------------------------------------
//	Build a 2 triangle face T(V0,V1,V2) and T(V0,V2,V3)
//	A positive orientable (CCW) is used
//------------------------------------------------------------------------------
bool COBJparser::Parse4Faces(char *s)
{	int v0,v1,v2,v3;
	int t0,t1,t2,t3;
	OBJ_TRIANGLE *T = 0;
	int nf	= sscanf(s,"f %d/%d  %d/%d %d/%d %d/%d", &v0,&t0,&v1,&t1,&v2,&t2,&v3,&t3);
	if (8 != nf)		return false;
	//--- Generate 2 Triangles -----------------
	bool ok = true;
	ok &= BuildTriangleVertex(0,v0,t0);
	ok &= BuildTriangleVertex(1,v1,t1);
	ok &= BuildTriangleVertex(2,v2,t2);
	T		= new OBJ_TRIANGLE();
	*T	= tri;
	mat->triQ.push_back(T);
	if (!ok)	 		return false;	
	//--- Generate the second triangle --------
	ok &= BuildTriangleVertex(0,v0,t0);
	ok &= BuildTriangleVertex(1,v2,t2);
	ok &= BuildTriangleVertex(2,v3,t3);
	T		= new OBJ_TRIANGLE();
	*T	= tri;
	mat->triQ.push_back(T);
	return ok;
}
//------------------------------------------------------------------------------
//	Build a 1 triangle face T(V0,V1,V2) and T(V0,V2,V3)
//	A positive orientable (CCW) is used
//------------------------------------------------------------------------------
bool COBJparser::Parse3Faces(char *s)
{	int v0,v1,v2;
	int t0,t1,t2;
	OBJ_TRIANGLE *T = 0;
	int nf	= sscanf(s,"f %d/%d  %d/%d %d/%d ", &v0,&t0,&v1,&t1,&v2,&t2);
	if (6 != nf)		return false;
	//--- Generate 1 Triangles -----------------
	bool ok = true;
	ok &= BuildTriangleVertex(0,v0,t0);
	ok &= BuildTriangleVertex(1,v1,t1);
	ok &= BuildTriangleVertex(2,v2,t2);
	T		= new OBJ_TRIANGLE();
	*T	= tri;
	mat->triQ.push_back(T);
	return ok;
}
//------------------------------------------------------------------------------
//	Build a 1 triangle face T(V0,V1,V2) and T(V0,V2,V3)
//	A positive orientable (CCW) is used
//------------------------------------------------------------------------------
bool COBJparser::Parse3NFaces(char *s)
{	int v0,v1,v2;
	int	n0,n1,n2;
	int t0,t1,t2;
	OBJ_TRIANGLE *T = 0;
	int nf	= sscanf(s,"f %d/%d/%d  %d/%d/%d %d/%d/%d ", &v0,&t0,&n0,&v1,&t1,&n1,&v2,&t2,&n2);
	if (9 != nf)		return false;
	//--- Generate 1 Triangles -----------------
	bool ok = true;
	ok &= BuildTriangleVertex(0,v0,t0,n0);
	ok &= BuildTriangleVertex(1,v1,t1,n1);
	ok &= BuildTriangleVertex(2,v2,t2,n2);
	T		= new OBJ_TRIANGLE();
	*T	= tri;
	mat->triQ.push_back(T);
	return ok;
}

//------------------------------------------------------------------------------
//	Build a Part for a 3D model
//	Not yet implemented because we must compute normal vector if they are
//	omitted to be consistent with the 3D objects
//------------------------------------------------------------------------------
void COBJparser::BuildW3DPart()
{	return;}
//------------------------------------------------------------------------------
//	Build a Part for current material
//------------------------------------------------------------------------------
C3DPart *COBJparser::BuildMATPart(char dir)
{	U_INT ntr = mat->triQ.size();
	int		nbv = ntr * 3;
	C3DPart *prt    = new C3DPart();
	prt->AllocateOsmGVT(nbv);
	//--- Copy all  vertices from triangles ----------------------
	GN_VTAB *dst = prt->GetGTAB();
	//------------------------------------------------------------
	for (U_INT k=0; k < ntr; k++)
	{	OBJ_TRIANGLE *Tr = mat->triQ[k];
		*dst++ = Tr->vtx[0];
		*dst++ = Tr->vtx[1];
		*dst++ = Tr->vtx[2];
	}
	//---- Get a texture reference -------------------------------
	txd.Dir = dir;
	txd.apx = 0xFF;
	txd.azp = 0x00;
	strncpy(txd.name,mat->name,64);
	CShared3DTex *ref	= globals->txw->Get3DTexture(txd);	//GetM3DPodTexture(txd);
	prt->SetTREF(ref);
	return prt;
}

//------------------------------------------------------------------------------
//	Build a list of Part for OSM object
//------------------------------------------------------------------------------
C3DPart *COBJparser::BuildOSMPart(char dir)
{	C3DPart *head = 0;
	C3DPart *prev	= 0;
	for (U_INT k=0; k<matQ.size(); k++)
	{	mat = matQ[k];
		C3DPart *prt = BuildMATPart(dir);
		if (0 == head)	head = prt;
		if (prev)		prev->SetNext(prt);
		prev = prt;
	}
	return head;
}

//------------------------------------------------------------------------------
//	Return a strip of vertices, ignoring the texture name
//------------------------------------------------------------------------------
int  COBJparser::TransformVerticeStrip(GN_VTAB **buf)
{	HTransformer HT(C,S,T,E);					// Init tranformer
	//-------------------------------------------------------------
	U_INT nbt = mat->triQ.size();
	int   nbv = nbt * 3;
	GN_VTAB *tab = new GN_VTAB[nbv];
	GN_VTAB *dst = tab;
	//------------------------------------------------------------
	for (U_INT k=0; k < nbt; k++)
	{	OBJ_TRIANGLE *Tr = mat->triQ[k];
		HT.ComputeRT(Tr->vtx[0],dst++);
		HT.ComputeRT(Tr->vtx[1],dst++);
		HT.ComputeRT(Tr->vtx[2],dst++);
	}
	*buf  = tab;
	return nbv;
}
//------------------------------------------------------------------------------
//	Extend the chain of Parts with new vertices
//	For each material, search a part to extend or create One
//------------------------------------------------------------------------------
void COBJparser::ExtendParts(C3DPart *P)
{	
}
//===================================END OF FILE ==========================================