/*
 * Fonts.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2004 Chris Wallace
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
 */

/*! \file Fonts.cpp
 *  \brief Implements Fly! bitmapped font support functions
 */

#include "../Include/FlyLegacy.h"
#include "../Include/Utility.h"
#include "../Include/Globals.h"
#include <vector>
using namespace std;


CBitmappedFontChar::CBitmappedFontChar (void)
{
  ascii = 0;
  start = end = 0;
  w = h = 0;
}

CBitmappedFontChar::~CBitmappedFontChar (void)
{ for (U_INT k = 0; k < bits.size(); k++) delete bits[k];
	bits.clear();
}

void CBitmappedFontChar::SetBit (int x, int y)
{
  SFontCharBit *bit = new SFontCharBit;
  bit->x = x;
  bit->y = y;
  bits.push_back (bit);
}

void CBitmappedFontChar::Draw (SSurface *surface, int x, int y, unsigned int colour)
{
  std::vector<SFontCharBit*>::iterator i;
  for (i=bits.begin(); i!=bits.end(); i++) {
    SFontCharBit *bit = *i;
    DrawDot (surface, (x + bit->x), (y + bit->y), colour);
  }
}

inline int CBitmappedFontChar::GetWidth (void) { return w; }
inline int CBitmappedFontChar::GetHeight (void) { return h; }




//====================================================================
// CBitmappedFont
//====================================================================

CBitmappedFont::CBitmappedFont (const char* fntFilename)
{
  // Initialize
 *rawFilename = 0;
  w = h = 0;
  start = end = 0;
  transparent = 255;
  space = 0;
  shadow_x = shadow_y = 0;
  chars = NULL;

  // If .FNT filename was supplied, re-initialize from it
  if (fntFilename && fntFilename[0]) {
    Load (fntFilename);
  }
}

CBitmappedFont::~CBitmappedFont (void)
{
  delete[] chars;
}


/*
 * Load in the details of a bitmapped font from the specified FNT file
 *  Example .FNT file (ART\FTTINY7W.FNT in Startup.pod):

-------- font bitmap name --------
-------- width, height -----------
126,68
-------- start, end range --------
32,126
-------- transparency index ------
0
-------- space width -------------
2
-------- shadow offset x,y -------
0,0

  */
int CBitmappedFont::Load (const char* fntFilename)
{
  int rc = 0;

  return rc;
}

//
// This method loads the bitmapped font image data from the specified .RAW
//   file.  All relevant members of the class instance must have been
//   initialized before calling this method.
//
int CBitmappedFont::LoadRaw (const char* rawFilename)
{
  int i, j, k, m;
  int rc = 0;

  // Load RAW image data
  PODFILE *pRaw = popen (&globals->pfs, rawFilename);
  if (pRaw != NULL) {
    // Allocate array of font characters
    int nChars = end - start + 1;
    chars = new CBitmappedFontChar[nChars];

    // Declare temporary storage for raw image data
    unsigned char *rawdata = new unsigned char[w * h];
    pread (rawdata, 1, (w * h), pRaw);
    pclose (pRaw);

    int iChar = 0;                // Index of current bitmapped font char
    int iRowStart = 0;            // Index to first char in a pixel row
    int iRowEnd = 0;              // Index to last char in a pixel row
    int iRowCount = 0;            // Pixel row count for each char
    unsigned char *p = rawdata;   // Pointer to next byte of RAW image data
    unsigned char ciHighlight = 0;          // Colour index value for highlight
    bool highlightSet = false;    // Whether highlight colour has been determined yet
    bool done = false;
    for (i=0; (i<h) && !done; i++) {
      // At start of each row, initialize:
      //    - previous colour index to background
      //    - start of char range for row to current char index
      //    - flag to indicate that the next character (if found) is first in the row
      unsigned char ciPrev = transparent;
      bool firstCharInRow = true;
      iChar = iRowStart;

      for (j=0; (j<w) && !done; j++) {
        // Get pixel colour index from raw data
        unsigned char ci = *p;
        p++;

        // Start tests against colour index
        if (ci != transparent) {
          // First check to see if highlight colour has been set.  If not, then
          //   this colour is the highlight colour
          if (!highlightSet) {
            ciHighlight = ci;
            highlightSet = true;
          }

          // Check if this colour is the highlight
          if (ci == ciHighlight) {
            // If the previous colour was background then this is the start of a new
            //   character marker
            if (ciPrev == transparent) {
              if (firstCharInRow) {
                // Start of a new row of highlights.  Set heights of all chars
                //   in the previous row (if applicable) and reset the row
                //   control variables
                for (m=iRowStart; m<iRowEnd; m++) {
                  chars[m].h = iRowCount;
                }

                // Check if all characters have been read
                if (iRowEnd >= nChars) {
                  done = true;
                  break;
                }

                // Initialize control variables for this row of chars
                iRowStart = iRowEnd;
                iRowCount = 0;
                iChar = iRowStart;
                firstCharInRow = false;
              }
              // Set current column as starting column for this character
              chars[iChar].ascii = (unsigned char)(start + iChar);
              chars[iChar].start = j;
              iRowEnd++;
            }
          } else {
            // Neither background nor highlight, this is a font image bit
            // Check for transition to subsequent char
            for (k = iRowStart; k < iRowEnd; k++) {
              if (j >= chars[k].start) iChar = k;
            }

            // Add bit to char
            if (j > chars[iChar].end) {
              WARNINGLOG ("Font bit out of range : %s @ (%d,%d), char=%d",
                rawFilename, j, i, iChar);
            } else {
              chars[iChar].SetBit (j - chars[iChar].start, iRowCount);
            }
          }
        } else {
          // Pixel is transparent (background colour).  If previous pixel was
          //   highlight, then we have just transitioned out of a character marker,
          //   otherwise ignore
          if (ciPrev == ciHighlight) {
            chars[iChar].end = j-1;
            chars[iChar].w = chars[iChar].end - chars[iChar].start + 1;
            iChar++;
            if (iChar > nChars) {
              gtfo ("CBitmappedFont : Font character %d out of range for %s",
                iChar, rawFilename);
            }
          }
        }
        ciPrev = ci;
      } // for j = 0..w

      // Check for highlight that ends at right hand side of image
      if ((ciPrev == ciHighlight) && (iChar < nChars)) {
        chars[iChar].end = j-1;
        chars[iChar].w = chars[iChar].end - chars[iChar].start + 1;
        iChar++;
      }

      if (iRowEnd > nChars) {
        gtfo ("CBitmappedFont : Font character %d out of range for %s",
          iChar, rawFilename);
      }

      iRowCount++;
    } // for i = 0..h

    delete rawdata;

    rc = 1;
  }
	adl = CharHeight('H') +2;
  return rc;
}
//----------------------------------------------------------------------------------
// Render bitmapped text onto the drawing surface using the specified font.
//   The text is left-justified with the upper-left corner at surface offset (x,y)
//----------------------------------------------------------------------------------
int CBitmappedFont::DrawNText (SSurface *sf,int x, int y, U_INT rgb, char* text)
{ cln	= x;
	lin	= y + adl;
	for (unsigned int i=0; text[i] != 0; i++) 
	{ U_CHAR ascii = text[i];
		//--- space process -----------------------
		if (ascii == 0x20) {x += space; continue;}
		//--- New line process --------------------
		if (ascii == 0x0A) {x=cln; y = lin; lin += adl; continue;}
		//--- Other characters --------------------
    CBitmappedFontChar *fc = GetAscii (ascii);
    if (fc == 0) continue;
    fc->Draw (sf, x, y, rgb);
    x += fc->GetWidth () + 1;
  }
  return x;
}
//----------------------------------------------------------------------------------
//  Draw a single character
//----------------------------------------------------------------------------------
int CBitmappedFont::DrawChar (SSurface *sf, int x, int y, char c,U_INT rgb)
{ if (c == ' ')         return space;
  CBitmappedFontChar *fc = GetAscii (c);
  if (fc != NULL) {
    fc->Draw (sf, x, y, rgb);
    return fc->GetWidth()+ 1;
  }
  return 0;
}
//----------------------------------------------------------------------------------
//  Draw text with a limited number of characters
//  NOTE:  Use only when text buffer is greater than the limit
//----------------------------------------------------------------------------------
int CBitmappedFont::DrawLimChar(SSurface *sf,int x, int y, int Nb, U_INT rgb, char* text)
{ char  save  = 0;
  int   lgr   = strlen(text);
  if (Nb && (Nb < lgr)) {save = text[Nb]; text[Nb] = 0;}
  int rc = DrawNText(sf, x, y, rgb, text);
  if (save) text[Nb]  = save;
  return rc; }

//----------------------------------------------------------------------------------
// Render bitmapped text onto the drawing surface using the specified font.
//   The text is left-justified with the upper-left corner at surface offset (x,y)
//   Only the first nb characters are drawn
//----------------------------------------------------------------------------------
int CBitmappedFont::DrawNbrChar(SSurface *sf,int x, int y, U_INT rgb,char* text, int nb)
{ 
  for (unsigned int i=0; nb != 0; i++) {
    unsigned char ascii = text[i];
    if (0 ==ascii)  return x;
    nb--;
    if (ascii == 0x20) {
      x += space;
      continue;
    } 
    CBitmappedFontChar *fc = GetAscii (ascii);
    if (0 == fc) continue;
    fc->Draw (sf, x, y, rgb);
    x += fc->GetWidth () + 1;
  }
  return x; 
}
//----------------------------------------------------------------------------------
// Render bitmapped text Verticaly onto the drawing surface using the specified font.
//   The text is left-justified with the upper-left corner at surface offset (x,y)
//----------------------------------------------------------------------------------
int CBitmappedFont::DrawTextV (SSurface *sf,int x, int y, U_INT rgb, char* text, int dh)
{for (unsigned int i=0; text[i] != 0; i++) {
    unsigned char ascii = text[i];
    CBitmappedFontChar *fc = GetAscii (ascii);
    if (fc != NULL) {
        fc->Draw (sf, x, y, rgb);
        y += fc->GetHeight() + dh;
      }
    
  }
return y;
}
//----------------------------------------------------------------------------------
// Render bitmapped text onto the drawing surface using the specified font.
//   The text is centered with the top-center at surface offset (x,y)
//----------------------------------------------------------------------------------
void CBitmappedFont::DrawTextC (SSurface *sf,int x, int y, U_INT rgb, char* text)
{ // Adjust x-offset by half of string length and call DrawNText
  int offset = TextWidth (text) / 2;
  DrawNText (sf, (x - offset), y, rgb, text);
}
//----------------------------------------------------------------------------------
// Render bitmapped text onto the drawing surface using the specified font.
//   The text is left-justified with the top-right corner at surface offset (x,y)
//----------------------------------------------------------------------------------
void CBitmappedFont::DrawTextR (SSurface *sf,int x, int y, U_INT rgb, char* text)
{ int offset = TextWidth (text);
  DrawNText (sf, (x - offset), y, rgb, text);
}
//----------------------------------------------------------------------------------
//  return text width
//----------------------------------------------------------------------------------
int CBitmappedFont::TextWidth (char* text)
{ int rc = 0;
  for (unsigned int i=0; text[i] != 0; i++) {
    // Increment by char length (plus 1 for inter-char space)
    rc += CharWidth (text[i]) + 1;
  }
  return rc;
}
//----------------------------------------------------------------------------------
//  return text height
//----------------------------------------------------------------------------------
int CBitmappedFont::TextHeight (char* text)
{
  int rc = 0;

  for (unsigned int i=0; text[i] != 0; i++) {
    int height = CharHeight (text[i]);
    if (height > rc) { rc = height; }
  }

  return rc;
}
//----------------------------------------------------------------------------------
//  return character height
//----------------------------------------------------------------------------------
int CBitmappedFont::CharHeight (U_CHAR c)
{ int rc = 0;
  if (IsCharValid (c)) rc = chars[c - start].h;
  return rc;
}
//----------------------------------------------------------------------------------
//  return character width
//----------------------------------------------------------------------------------
int CBitmappedFont::CharWidth (U_CHAR c)
{ int rc = 0;

  if (c == ' ') {
    rc = space;
  } else if (IsCharValid (c)) {
    rc = chars[c - start].w;
  }

  return rc;
}
//------------------------------------------------------------------------------------
inline bool CBitmappedFont::IsCharValid (unsigned char c)
{
  return ((c >= start) && (c <= end));
}
//------------------------------------------------------------------------------------

CBitmappedFontChar *CBitmappedFont::GetAscii (unsigned char c)
{
  return (IsCharValid (c))?(&(chars[c - start])):(0);
}
//------------------------------------------------------------------------------------
typedef struct {
  char      rawFilename[64];
  int       w, h;
  int       start, end;
  int       space;
} SDefaultFont;

SDefaultFont defaultFonts[] =
{
  //     RAW Filename      w       h     start   end    space
  {"ART/FTASCI10.RAW",   272,    188,    33,    255,     5},
  {"ART/FTASCM10.RAW",   224,    224,    33,    222,     5},
  {"ART/FTDIGI9.RAW",     72,    110,    33,    133,     6},
  {"ART/FTDIGI9B.RAW",    69,     41,    45,     90,     6},
  {"ART/FTDIGI9D.RAW",    69,     41,    45,     90,     6},
  {"ART/FTDIGI13.RAW",    97,    130,    33,    134,     8},
  {"ART/FTDOS12.RAW",    221,     79,    33,    126,     7},
  {"ART/FTMICRO5.RAW",   100,    100,    33,    126,     4},
  {"ART/FTMNO14B.RAW",   193,     97,    33,    126,    12},
  {"ART/FTMONO12.RAW",   222,     79,    33,    126,     7},
  {"ART/FTNORM14.RAW",   275,    147,    33,    126,    14},
  {"ART/FTNORM28.RAW",   463,    238,    33,    126,    22},
  {"ART/FTSMAL10.RAW",   162,     73,    33,    127,     8},
  {"ART/FTTHIN24.RAW",   215,    151,    33,    126,     6},
  {"ART/FTTINY7.RAW",    126,     68,   33,     126,     5},
  {"ART/FTTINY7W.RAW",   126,     68,   33,     126,     5},
};

//=====================================================================================
// Load the bitmapped font.  The font->fontName attribute of the SFont struct
//   must be populated with the name of the .RAW font image data, including
//   the pod filesystem path name (typically Art/)
//
// Return code:
//  0 (false) = Error encountered during font loading
//  1 (true)  = Font loaded successfully
//=====================================================================================
int LoadFont (SFont *font)
{
  int i;
  int rc = 0;

  if (font == NULL) return rc;

  // Initialize *font member of SFont class
  font->font = NULL;

  // If this is a default font, create "empty" CBitmappedFont class instance,
  //   populate the members from the predefined table above, and load
  //   the RAW data.
  int nDefaultFonts = sizeof(defaultFonts) / sizeof (SDefaultFont);
  for (i=0; i<nDefaultFonts; i++) {
    SDefaultFont *d = &(defaultFonts[i]);
    if (stricmp (d->rawFilename, font->fontName) == 0) {
      // Instantiate blank SBitmappedFont
      CBitmappedFont *newfont = new CBitmappedFont ();

      // Populate with required data
      strncpy (newfont->rawFilename, font->fontName,63);
      newfont->w = d->w;
      newfont->h = d->h;
      newfont->start = d->start;
      newfont->end = d->end;
      newfont->space = d->space;

      // Store CBitmappedFont pointer in return struct
      font->font = newfont;
      
      // Load image data for font
      newfont->LoadRaw (font->fontName);
      rc = 1;
      break;
    }
  }
  /// \todo If not a default font then attempt to load the custom font
  if (font->font == NULL) {
  }

  return rc;
}

//===================================================================================
// Render a list of bitmapped text onto the drawing surface using the specified font.
//   The text is left-justified with the upper-left corner at surface offset (x,y)
//   Only the first nb characters are drawn
//===================================================================================
void DrawTextList(SSurface *sf,int x0, int y0,TXT_LIST *sT,U_SHORT Nb)
{ SFont *font = sT->font;
  U_INT  col  = sT->color;
  CMonoFontBMP *m14 =(CMonoFontBMP *)font->font;      // Mono font 14
  char  *text = 0;
  U_CHAR  car = 0;
  for (short nc = 0; nc < Nb; nc++)
  { text  = sT->GetNextAddr(text);
    car   = *text++;
    if (0 == car)                     break;
    if (car == 0x20) {x0 += m14->Space(); continue;}
    x0 += m14->DrawChar(sf,x0,y0,car,col);
  }
  sT->Reset();
  return;
}

//=================================================================================
//  FONT IN BMP FORMAT
//=================================================================================
//-----------Mono font in BMP (8 bits palettized) -----------
SDefaultFont MonoFont[] =
{
  {"ART/FTMONO8.BMP",         6,     10,    32,    155,     6},    // Index 0
  {"ART/FTMONO14.BMP",        9,     15,    32,    146,     9},    // Index 1
  {"ART/FTMONO20.BMP",       13,     20,    32,    146,    13},    // Index 2
  {"ART/FTLUCICONSOLE12.BMP", 7,     12,    32,    146,     7},    // Index 3    
};
//-----------Varifont in BMP (8 bits palettized) -----------
SDefaultFont VariFont[] =
{
  {"ART/FTRADI9.BMP",        0,     0,    32,    100,     0},    // Index 0
};
//================================================================================
//  Global function to load mono bitmap font
//================================================================================
int  LoadMonoFont(int inx,SFont *fnt)
{ short c1 = MonoFont[inx].start;
  short cf = MonoFont[inx].end;
  strncpy(fnt->fontName,MonoFont[inx].rawFilename,64);
  CMonoFontBMP *font =  new CMonoFontBMP(fnt->fontName,c1,cf);
  fnt->font = font;
  return 1;
}
//================================================================================
//  Global function to load vari bitmap font
//================================================================================
int  LoadVariFont(int inx,SFont *fnt)
{ short c1 = VariFont[inx].start;
  short cf = VariFont[inx].end;
  strncpy(fnt->fontName,VariFont[inx].rawFilename,64);
  CVariFontBMP *font =  new CVariFontBMP(fnt->fontName,c1,cf);
  fnt->font = font;
  return 1;
}
//------------------------------------------------------------------------------
//  Constructor 
//------------------------------------------------------------------------------
CMonoFontBMP::CMonoFontBMP (const char *fname,short from,short to)
{
  // Initialize
  image = NULL;
  wd = wb = ht = 0;
  mask  = 0xFFFFFFFF;
  start = from;
  end   = to;
  // Try to load from BMP file in POD filesystem
  PODFILE* p = popen (&globals->pfs, fname);
  if (0 == p) gtfo("Cant load font: %s",fname);
  Load (p);
  pclose (p);
}
//------------------------------------------------------------------------------
//  Destroy the font
//------------------------------------------------------------------------------
CMonoFontBMP::~CMonoFontBMP()
{ if (image) delete image;
}
//------------------------------------------------------------------------------
//  Load the font
//------------------------------------------------------------------------------
void CMonoFontBMP::Load (PODFILE *p)
{
  // Read header
  pread (&h, sizeof(SBmpHeader), 1, p);

  // Read info header
  pread (&ih, sizeof(SBmpInfoHeader), 1, p);

  // Set width/height
  wd = short(ih.w);
  ht = short(ih.h);
  //-------Set wide to word boundary ---------------
  int siz = wd;
  int mod = wd % 4;
  if (mod)  wd += (4 - mod);

  //----- Read image data---------------------------
  switch (ih.compression) {
  case 0:
    // Uncompressed
    { image = new char[wd * ht];
      pseek (p,h.offset,0);
      pread (image, wd * ht, 1, p);
    }
    break;

  default:
    // Only uncompressed images are currently supported
    WARNINGLOG ("CMonoFontBMP : Unsupported compression mode %d in %s",
      ih.compression, p->filename);
  }
  //------Compute font wide and height ------------------
  int dim = end - 1;
  wb  = wd;
  wd  = siz / dim;
	adl = CharHeight('H') +2;
}
//----------------------------------------------------------------------------------
//  return text width
//----------------------------------------------------------------------------------
int CMonoFontBMP::TextWidth (char* text)
{ int lg = strlen(text);
  return lg * wd;
}
//----------------------------------------------------------------------------------
//  return text height
//----------------------------------------------------------------------------------
int CMonoFontBMP::TextHeight (char* text)
{ return ht;}
//--------------------------------------------------------------------------------
//  Draw a character on the surface
//--------------------------------------------------------------------------------
int CMonoFontBMP::DrawChar(SSurface *sf,int x,int y,char c,U_INT rgba)
{ U_CHAR k  = (c)?((U_CHAR)c - start):(0);
  if (k >= end)  return 0;
  //----Compute start position in surface ---------------------------
  short xs  = (x < 0)?(0):(x);                            // Screen column
  if (xs >= short(sf->xSize))   return wd;                // Clip. Bitmap all to the right of surface
  short ys  = sf->ySize - y - ht;                         // Screen line
  if (ys >= short(sf->ySize))   return 0;                 // Clip. Bitmap all below the surface
  //---Compute start position in bitmap -----------------------------
  short xb  = (x < 0)?(-x):(0);
  if (xb >= wd)                 return wd;                // Clip. Bitmap all to the left of  surface
  short yb  = (ys < 0)?(-ys):(0);
  if (yb >= ht)                 return 0;                 // Clip. Bitmap all above surface
  if (ys < 0) ys = 0;                                     // base line always positive
  //---Compute number of line and column ----------------------------
  short dx1 = (sf->xSize - xs);                           // screen wide
  short dx2 = (wd - xb);                                  // bitmap wide
  short nc  = (dx1 < dx2)?(dx1):(dx2);
  short dy1 = (sf->ySize - ys);                           // screen height
  short dy2 = (ht - yb);                                  // bitmap height
  short nl  = (dy1 < dy2)?(dy1):(dy2);
  //---Buffer correction factor -------------------------------------
  short cs  = (sf->xSize - nc);
  short cb  = (wb - nc);
  //---Buffer starting positions ------------------------------------
  U_INT *bus = sf->drawBuffer + (ys * sf->xSize) + xs;
  char  *bub = image + (k * wd) + (wb * yb) + xb;
  //---Draw the character -------------------------------------------
  U_CHAR bit;
  while (nl--)
  { short cl = nc;
    while (cl--)
    {  bit    = *bub++; 
      *bus++  = (bit)?(rgba):(*bus & mask);
    }
    bus += cs;
    bub += cb;
  }
  return nc;
}
//--------------------------------------------------------------------------------
//  Draw Transparent text
//--------------------------------------------------------------------------------
int CMonoFontBMP::DrawNText (SSurface *sf,int x, int y, U_INT rgba,char* text)
{ if (0 == image) return 0;
	cln = x;
	lin	= y + adl;
  for (int i=0; text[i] != 0; i++)
	{ if (text[i] != 0x0A)	{x += DrawChar(sf,x,y,text[i],rgba);}
		else	{ x=cln; y=lin; lin += adl;}
	}
  return x;
}
//--------------------------------------------------------------------------------
//  Draw Full opaque text
//--------------------------------------------------------------------------------
void CMonoFontBMP::DrawFText (SSurface *sf,int x, int y, U_INT rgba,char* text)
{ if (0 == image) return;
  mask  = 0;
  for (int i=0; text[i] != 0; i++) x += DrawChar(sf,x,y,text[i],rgba);
  mask  = 0xFFFFFFFF;
  return;
}
//--------------------------------------------------------------------------------
//  Draw text with a limited char count
//--------------------------------------------------------------------------------
void CMonoFontBMP::DrawNText (SSurface *sf,int x, int y, U_INT rgba,char* text,int nc)
{ if (0 == image) return;
  for (int i=0; (text[i] != 0) && (nc != 0); i++) 
  {x += DrawChar(sf,x,y,text[i],rgba);
   nc--;
  }
  return;
}
//--------------------------------------------------------------------------------
//  Draw text with a limited char count replace space with _
//--------------------------------------------------------------------------------
void CMonoFontBMP::DrawSText (SSurface *sf,int x, int y, U_INT rgba,char* text,int nc)
{ char  k;
  if (0 == image) return;
  for (int i=0; (text[i] != 0) && (nc != 0); i++) 
  { k = text[i];
    if  (' ' == k)  k = '_';
    x += DrawChar(sf,x,y,k,rgba);
    nc--;
  }
  return;
}
//--------------------------------------------------------------------------------
// Render bitmapped text onto the drawing surface using the specified font.
//   The text is centered with the top-center at surface offset (x,y)
//--------------------------------------------------------------------------------
void CMonoFontBMP::DrawTextC (SSurface *sf,int x, int y, U_INT rgba,char* text)
{ int xm = (sf->xSize >> 1);
  if (0 == image)         return;
  int offset = (wd * strlen(text)) >> 1;
  DrawNText (sf,(xm - offset), y, rgba, text);
}
//--------------------------------------------------------------------------------
//  Draw a repeated char
//--------------------------------------------------------------------------------
void  CMonoFontBMP::ReptChar(SSurface *sf, int x0, int y0, int x1, U_INT rgba,char ascii)
{ if (0 == image)         return;
  while (x0 < x1)  x0 += DrawChar(sf,x0,y0,ascii,rgba);
  return;
}
//--------------------------------------------------------------------------------
//  Draw a number of characters
//--------------------------------------------------------------------------------
void CMonoFontBMP::DrawNText (SSurface *sf,int nc,int x, int y, U_INT rgba,char* text)
{ if (0 == image) return;
  for (int i=0; text[i] != 0; i++) 
  { x += DrawChar(sf,x,y,text[i],rgba);
    if (0 == --nc)  return;
  }
  return;
}
//===================================================================================
//  CLASS CVariFontBMP for variable character pace
//===================================================================================
//------------------------------------------------------------------------------
//  Constructor 
//------------------------------------------------------------------------------
CVariFontBMP::CVariFontBMP (const char *fname,short c1,short cf)
{
  // Initialize
  image   = 0;
  imgAD   = 0;
  DefTAB  = 0;
  start   = c1;
  end     = cf;
  wd = wb = ht = 0;
  mask  = 0xFFFFFFFF;
  // Try to load from BMP file in POD filesystem
  PODFILE* p = popen (&globals->pfs, fname);
  if (p != NULL) {
    Load (p);
    pclose (p);
  }
}
//------------------------------------------------------------------------------
//  Destroy the font
//------------------------------------------------------------------------------
CVariFontBMP::~CVariFontBMP()
{ if (imgAD)  delete imgAD;
  if (DefTAB) delete DefTAB;
}
//------------------------------------------------------------------------------
//  Buid the Definition table
//------------------------------------------------------------------------------
void CVariFontBMP::BuildDefTAB(short lgr, char *fn)
{ int dim     = end - 1;
  DefTAB      = new VCHAR_DEF[dim];           // Allocate definition
  char *buf   = image;                        // Dot line
  U_SHORT ofs = 0;                            // Offset
  U_SHORT pof = 0;                            // Previous offset
  int   No    = 0;                            // slot index
  while (No != dim)
  { while (0 == *buf++) {ofs++; continue;}
    char  ch = No + start;
    DefTAB[No].offset = pof;
    DefTAB[No].wide   = (ofs - pof);
//
#ifdef _DEBUG	
    TRACE("%s: CHAR %c No %02d at %03d- WD=%03d",fn,ch,No,pof,(ofs - pof));
#endif
    pof = ofs;
    ofs++;
    No++;
  }
  ht    -= 1;                               // Adjust char height
  image += lgr;                             // Bypass dot line
  return;
}
//------------------------------------------------------------------------------
//  Load the font
//------------------------------------------------------------------------------
void CVariFontBMP::Load (PODFILE *p)
{
  // Read header
  pread (&h, sizeof(SBmpHeader), 1, p);

  // Read info header
  pread (&ih, sizeof(SBmpInfoHeader), 1, p);

  // Set width/height
  wd = short(ih.w);
  ht = short(ih.h);
  //-------Set wide to word boundary ---------------
  int mod = wd % 4;
  if (mod)  wd += (4 - mod);

  //----- Read image data---------------------------
  switch (ih.compression) {
  case 0:
    // Uncompressed
    { image = new char[wd * ht];
      imgAD = image;
      pseek (p,h.offset,0);
      pread (image, wd * ht, 1, p);
    }
    break;

  default:
    // Only uncompressed images are currently supported
    WARNINGLOG ("CVariFontBMP : Unsupported compression mode %d in %s",
      ih.compression, p->filename);
  }
  wb  = wd;
  BuildDefTAB(wd,p->filename);
	adl = CharHeight('H') + 2;
}
//--------------------------------------------------------------------------------
//  Draw a character on the surface
//--------------------------------------------------------------------------------
int CVariFontBMP::DrawChar(SSurface *sf,int x,int y,char c,U_INT rgba)
{ U_CHAR k  = (c)?((U_CHAR)c - start):(0);
  if (k >= end)  return 0;
  short ofs = DefTAB[k].offset;
  short kwd = DefTAB[k].wide;
  //----Compute start position in surface ---------------------------
  short xs  = (x < 0)?(0):(x);                            // Screen column
  if (xs >= short(sf->xSize))   return wd;                // Clip. Bitmap all to the right of surface
  short ys  = sf->ySize - y - ht;                         // Screen line
  if (ys >= short(sf->ySize))   return 0;                 // Clip. Bitmap all below the surface
  //---Compute start position in bitmap -----------------------------
  short xb  = (x < 0)?(-x):(0);
  if (xb >= kwd)                return kwd;               // Clip. Bitmap all to the left of  surface
  short yb  = (ys < 0)?(-ys):(0);
  if (yb >= ht)                 return 0;                 // Clip. Bitmap all above surface
  if (ys < 0) ys = 0;                                     // base line always positive
  //---Compute number of line and column ----------------------------
  short dx1 = (sf->xSize - xs);                           // screen wide
  short dx2 = (kwd - xb);                                 // bitmap wide
  short nc  = (dx1 < dx2)?(dx1):(dx2);
  short dy1 = (sf->ySize - ys);                           // screen height
  short dy2 = (ht - yb);                                  // bitmap height
  short nl  = (dy1 < dy2)?(dy1):(dy2);
  //---Buffer correction factor -------------------------------------
  short cs  = (sf->xSize - nc);
  short cb  = (wb - nc);
  //---Buffer starting positions ------------------------------------
  U_INT *bus = sf->drawBuffer + (ys * sf->xSize) + xs;
  char  *bub = image + (ofs) + (wb * yb) + xb;
  //---Draw the character -------------------------------------------
  U_CHAR bit;
  while (nl--)
  { short cl = nc;
    while (cl--)
    {  bit    = *bub++; 
      *bus++  = (bit)?(rgba):(*bus & mask);
    }
    bus += cs;
    bub += cb;
  }
  return nc;
}
//--------------------------------------------------------------------------------
//  Return character wide
//--------------------------------------------------------------------------------
int CVariFontBMP::CharWidth(U_CHAR c)
{ U_CHAR k  = (c)?((U_CHAR)c - start):(0);
  short kwd = DefTAB[k].wide;
  return kwd;
}
//--------------------------------------------------------------------------------
//  Return width of text text
//--------------------------------------------------------------------------------
int CVariFontBMP::TextWidth  (char* text)
{ int wd = 0;
  char *src = text;
  while (*src) wd += CharWidth(*src++);
  return wd;
}
//--------------------------------------------------------------------------------
//  Draw Transparent text
//--------------------------------------------------------------------------------
int CVariFontBMP::DrawNText (SSurface *sf,int x, int y, U_INT rgba,char* text)
{ if (0 == image) return 0;
  if (0 == text)	return 0;
	cln	= x;
	lin = y + adl;
  for (int i=0; text[i] != 0; i++)
	{	if (text[i] != 0x0A)	x += DrawChar(sf,x,y,text[i],rgba);
	  else	{x = cln; y=lin; lin += adl;}
	}
  return x;
}
//====================================================================================
// Unload (free) a font when it is no longer needed
//====================================================================================
void FreeFont (SFont *font)
{ if (font == NULL) return;
  CBitmappedFont *f = (CBitmappedFont *)font->font;
  delete f;
  font->font = NULL;
}
//====================================================================================
//  Free BMP mono font
//====================================================================================
void FreeMonoFont(SFont *font)
{ if (font == NULL) return;
  CMonoFontBMP  *f = (CMonoFontBMP*)(font->font);
  delete f;
  font->font = 0;
  return;
}
//====================================================================================
//  Free BMP variable font
//====================================================================================
void FreeVariFont(SFont *font)
{ if (font == NULL) return;
  CVariFontBMP  *f = (CVariFontBMP*)(font->font);
  delete f;
  font->font = 0;
  return;
}
//=============================END OF FILE =======================================