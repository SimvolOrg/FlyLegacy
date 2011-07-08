/*
 * Portability.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003-2005 Chris Wallace
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

/**
 * @file Portability.cpp
 * @brief Implements platform portability functions
 */

#include "../Include/Utility.h"

#ifndef HAVE__STRUPR
char *strupper (char *s)
{
  for (unsigned int i=0; i<strlen(s); i++) s[i] = (char)toupper(s[i]);
  return s;
} 
#endif // HAVE__STRUPR

#ifndef HAVE_STRREV
char *strrev(char *str)
{
  char *p1, *p2;

  if (! str || ! *str)
    return str;
  for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
  {
    *p1 ^= *p2;
    *p2 ^= *p1;
    *p1 ^= *p2;
  }
  return str;
}
#endif // HAVE_STRREV

#ifndef HAVE_ITOA
char* itoa(int value, char*  str, int radix)
{
  int  rem = 0;
  int  pos = 0;
  char ch  = '!' ;
  do
  {
    rem    = value % radix ;
    value /= radix;
    if ( 16 == radix )
    {
      if( rem >= 10 && rem <= 15 )
      {
        switch( rem )
        {
          case 10:
            ch = 'a' ;
            break;
          case 11:
            ch ='b' ;
            break;
          case 12:
            ch = 'c' ;
            break;
          case 13:
            ch ='d' ;
            break;
          case 14:
            ch = 'e' ;
            break;
          case 15:
            ch ='f' ;
            break;
        }
      }
    }
    if( '!' == ch )
    {
      str[pos++] = (char) ( rem + 0x30 );
    }
    else
    {
      str[pos++] = ch ;
    }
  }
  while( value != 0 );
  str[pos] = '\0' ;
  return strrev(str);
}
#endif // HAVE_ITOA
