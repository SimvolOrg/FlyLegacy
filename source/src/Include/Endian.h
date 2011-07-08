/*
 * Endian.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2005 Chris Wallace
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

/*! \file Endian.h
 *  \brief Declaration of Endian functions
 */

#ifndef ENDIAN_H
#define ENDIAN_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
// Local functions to swap specific data sizes regardless of type
//

static inline void SwapBytes (void *p, unsigned int nBytes)
{
  unsigned char *s = (unsigned char*)p;
  for (unsigned int i=0; i<(nBytes/2); i++) {
    unsigned char temp = s[i];
    s[i] = s[nBytes - i - 1];
    s[nBytes - i - 1] = temp;
  }
}

// @todo Remove this if generic SwapBytes is sufficient
static inline void Swap16 (void *p)
{
  SwapBytes (p, 2);
/*
  *p = ((*p << 8) & 0xFF00) |
        (*p >> 8) & 0x00FF));
*/
}

static inline void Swap32 (void *p)
{
  SwapBytes (p, 4);
/*
  *p = ((*p >> 24) & 0x000000FF) | 
       ((*p >>  8) & 0x0000FF00) | 
       ((*p <<  8) & 0x00FF0000) | 
       ((*p << 24) & 0xFF000000);
*/
}

static inline void Swap64 (void *p)
{
  SwapBytes (p, 8);
/*
  int nBytes = 8;
  unsigned char *s = (unsigned char*)p;
  for (int i=0; i<(nBytes/2); i++) {
    unsigned char temp = s[i];
    s[i] = s[nBytes - i - 1];
    s[nBytes - i - 1] = temp;
  }
*/
}


//
// Swap Endian-ness of specific data types
//
inline unsigned int  SwapEndian (unsigned int v)
{
  SwapBytes (&v, sizeof(unsigned int));
  return v;
}

inline int           SwapEndian (int v)
{
  SwapBytes (&v, sizeof(int));
  return v;
}

inline unsigned short  SwapEndian (unsigned short v)
{
  Swap16 (&v);
  return v;
}

inline short           SwapEndian (short v)
{
  Swap16 (&v);
  return v;
}

inline unsigned long   SwapEndian (unsigned long v)
{
  Swap32 (&v);
  return v;
}

inline long            SwapEndian (long v)
{
  Swap32 (&v);
  return v;
}

inline float           SwapEndian (float v)
{
  Swap32 (&v);
  return v;
}

inline double          SwapEndian (double v)
{
  Swap64 (&v);
  return v;
}


//
// Functions to determine native machine format
//

static const int EndianTest = 1;

inline bool IsBigEndian (void)
{
  return (*((char *) &EndianTest ) == 0);
}

inline bool IsLittleEndian (void)
{
  return (*((char *) &EndianTest ) != 0);
}


//
// Convert little Endian data to native machine format
//

inline unsigned int  LittleEndian (unsigned int v)
{
  if (IsLittleEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline int           LittleEndian (int v)
{
  if (IsLittleEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline unsigned short  LittleEndian (unsigned short v)
{
  if (IsLittleEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline short           LittleEndian (short v)
{
  if (IsLittleEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline unsigned long   LittleEndian (unsigned long v)
{
  if (IsLittleEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline long            LittleEndian (long v)
{
  if (IsLittleEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline float           LittleEndian (float v)
{
  if (IsLittleEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline double          LittleEndian (double v)
{
  if (IsLittleEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}


//
// Convert big Endian data to native machine format
//

inline unsigned int  BigEndian (unsigned int v)
{
  if (IsBigEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline int           BigEndian (int v)
{
  if (IsBigEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline unsigned short  BigEndian (const unsigned short v)
{
  if (IsBigEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline short           BigEndian (const short v)
{
  if (IsBigEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline unsigned long   BigEndian (const unsigned long v)
{
  if (IsBigEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline long            BigEndian (const long v)
{
  if (IsBigEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline float           BigEndian (const float v)
{
  if (IsBigEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

inline double          BigEndian (const double v)
{
  if (IsBigEndian()) {
    return v;
  } else {
    return SwapEndian (v);
  }
}

#endif // ENDIAN_H
