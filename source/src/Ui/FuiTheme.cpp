/*
 * FuiTheme.cpp
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
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

/*! \file FuiTheme.cpp
 *  \brief Implements CFuiTheme and related classes for FUI themes
 */

#include "../Include/Fui.h"
#include "../Include/Globals.h"

using namespace std;

//==================================================================================
// CFuiThemeWidget
//===================================================================================
CFuiThemeWidget::CFuiThemeWidget (void)
{ black = MakeRGBA(0,0,0,255);
}
//------------------------------------------------------------------
//	Destructor 
//------------------------------------------------------------------
CFuiThemeWidget::~CFuiThemeWidget (void)
{ // Free bitmaps
  std::map<string,CBitmap*>::iterator i;
  for (i=bmMap.begin(); i!=bmMap.end(); i++) delete (*i).second;
  bmMap.clear();
  // Free other maps
  colourMap.clear();
  componentNameMap.clear();
	flagSet.clear();
}

void CFuiThemeWidget::ParseBitmap (char* s)
{
  // First token is element type
  const char *delimiters = " \t";
  char *token  = strtok (NULL, delimiters);
  char *bmName = strtok (NULL, delimiters);
	char *nFrame = strtok (NULL, delimiters);
	
  // Check if map entry already exists
  std::map<string,CBitmap*>::iterator i = bmMap.find(token);
  if (i == bmMap.end()) {
    // Get bitmap name and load
    CBitmap *bm = new CBitmap(bmName);
		if (nFrame) bm->SetFrameNb(atoi(nFrame));
    if (bm->NotLoaded()) {
      // ERROR
      WARNINGLOG ("CFuiThemeWidget : Invalid bitmap %s", bmName);
  		SAFE_DELETE(bm);
    } else {
      // Add the bitmap to the map
      bmMap[token] = bm;
    }
  } else {
    // Generate warning for duplicate theme entry
    WARNINGLOG ("CFuiThemeWidget : Duplicate bitmap tag %s, value %s ignored", token, bmName);
  }
}

void CFuiThemeWidget::ParseColour (char* s)
{
  // First token is element name
  const char *delimiters = " \t";
  char *token = strtok (NULL, delimiters);
  string elementName = token;

  // Next tokens are R, G, B colour values
  token = strtok (NULL, delimiters);
  int r = atoi (token);
  token = strtok (NULL, delimiters);
  int g = atoi (token);
  token = strtok (NULL, delimiters);
  int b = atoi (token);

  colourMap[elementName] = MakeRGB (r, g, b);
}

void CFuiThemeWidget::ParseFrameRate (char *s)
{
  // First token is element type
  const char *delimiters = " \t";
  char *token = strtok (NULL, delimiters);
  framerate = atoi (token);
}

void CFuiThemeWidget::ParseThickness (char *s)
{
  // First token is element type
  const char *delimiters = " \t";
  char *token = strtok (NULL, delimiters);
  thickness = atoi (token);
}

void CFuiThemeWidget::ParseComponentName (char *s, string type)
{
  // First token is element type
  const char *delimiters = " \t";
  char *token = strtok (NULL, delimiters);
  componentNameMap[type] = token;
}

void CFuiThemeWidget::ParseButtonWidth (char *s)
{
  // First token is element type
  const char *delimiters = " \t";
  char *token = strtok (NULL, delimiters);
  buttonWidth = atoi (token);
}

void CFuiThemeWidget::SetFlag (string name)
{
  flagSet.insert (name);
}
//===========================================================================
void CFuiThemeWidget::SetName (const char* s)
{
  strncpy (name, s,63);
}
//===========================================================================
CBitmap *CFuiThemeWidget::GetBitmap (string name)
{ return bmMap[name];
}
//------------------------------------------------------------
//  Return text color or black by default
//------------------------------------------------------------
unsigned int CFuiThemeWidget::GetColour (string name)
{ U_INT col = colourMap[name];
  return (col)?(col):(black); }
//------------------------------------------------------------
//  Return component name
//------------------------------------------------------------
const char *CFuiThemeWidget::GetComponentName (string name)
{
  return componentNameMap[name].c_str();
}

bool CFuiThemeWidget::GetFlag (string name)
{
  bool rc = false;

  set<string>::iterator i;
  for (i=flagSet.begin(); i!=flagSet.end(); i++) {
    if (i->compare(name) == 0) {
      rc = true;
      break;
    }
  }

  return rc;
}

void CFuiThemeWidget::Print (FILE *f)
{
  // Print bitmap elements
  std::map<string,CBitmap*>::iterator i;
  for (i=bmMap.begin(); i!=bmMap.end(); i++) {
    CBitmap *bm = i->second;
    fprintf (f, "\tBITMAP\t%s\t%s\n", i->first.c_str(), bm->name());
  }
  
  // Print colour elements
  std::map<string,unsigned int>::iterator j;
  for (j=colourMap.begin(); j!=colourMap.end(); j++) {
    unsigned int r, g, b;
    UnmakeRGB (j->second, &r, &g, &b);
    fprintf (f, "\tCOLOR\t%s\t%d %d %d\n", j->first.c_str(), r, g, b);
  }

  // Print flags
  set<string>::iterator k;
  for (k=flagSet.begin(); k!=flagSet.end(); k++) {
    fprintf (f, "\t%s\n", k->c_str());
  }

  // Print sub-component names
  std::map<string,string>::iterator m;
  for (m=componentNameMap.begin(); m!=componentNameMap.end(); m++) {
    fprintf (f, "\t%s\t%s\n", m->first.c_str(), m->second.c_str());
  }
}


//===========================================================================
// CFuiTheme
//===========================================================================
CFuiTheme::CFuiTheme (void)
{*name = 0;
  id = 0;
}
//===========================================================================
//	Constructor 
//===========================================================================
CFuiTheme::CFuiTheme (const char *themeFilename)
{*name = 0;
  id = 0;
   // static?

  // Open .SCH file and parse lines
  PODFILE *p = popen (&globals->pfs, themeFilename);
  if (p != NULL) {
    // Begin parsing lines
    char s[256];
    pgets (s, 256, p);
    const char *delimiters = " \t";
    while (!peof(p)) {
  	  if ((strncmp (s, "\r\n", 2) == 0)) {	// empty line
          pgets (s, 256, p);
          continue;
  	  };

      // Trim trailing whitespace
      TrimTrailingWhitespace (s);

      // Check for comment or blank line
      if ((strncmp (s, "//", 2) == 0) || (strlen(s) == 0)) {
        pgets (s, 256, p);
        continue;
      }
        
      // Check first token
      char *token = strtok (s, delimiters);
      if (stricmp (token, "THEMENAME") == 0) {
        // Get the theme user-visible name
        token = strtok (NULL, "");
        strncpy (this->name, token,63);
      } else 
			if (stricmp (token, "THEMEID") == 0) {
        // Get the theme unique identifier
        token = strtok (NULL, delimiters);
        this->id = StringToTag (token);
			} else 
			if (stricmp (token, "WIDGET") == 0) {
        // Set the active widget
        token = strtok (NULL, delimiters);
        activeWidget = token;
        if (widgetMap.find(activeWidget) != widgetMap.end()) // Widget is already defined
          WARNINGLOG ("CFuiTheme : Duplicate widget type %s", activeWidget);
        widgetMap[activeWidget] = new CFuiThemeWidget;

			} else	
			
      // If the active widget is valid, parse the element type and allow
      //   the widget to parse the details
			if (0 == Decode(token,s))	break;
     // Get next line
		
      pgets (s, 256, p);
		}
    pclose (p);
  } else {
    gtfo ("CFuiTheme : Cannot open theme file %s", themeFilename);
  }
}
//======================================================================================
CFuiTheme::~CFuiTheme (void)
{
  std::map<string,CFuiThemeWidget*>::iterator i;
  for (i=widgetMap.begin(); i!=widgetMap.end(); i++) delete (*i).second;
  widgetMap.clear();
}
//======================================================================================
CFuiThemeWidget *CFuiTheme::GetWidget (string name)
{
  CFuiThemeWidget *rc = NULL;

  std::map<string,CFuiThemeWidget*>::iterator i = widgetMap.find(name);
  if (i != widgetMap.end()) {
    rc = i->second;
  }

  return rc;
}
//==============================================================================
//	Decode the line
//=================================================================================
int CFuiTheme::Decode(char *t,char *s)
{ if (activeWidget.empty())	return 1;;
	CFuiThemeWidget *widget = widgetMap[activeWidget];
	if (stricmp (t, "BITMAP") == 0)			{widget->ParseBitmap (s);		return 1;}
	if (stricmp (t, "COLOR") == 0)			{widget->ParseColour (s);		return 1;}
	if (stricmp (t, "USESHADOW") == 0)	{widget->SetFlag (t);				return 1;}
	if (stricmp (t, "FRAMERATE") == 0)	{widget->ParseFrameRate(s);	return 1;}
	if (stricmp (t, "THICKNESS") == 0)	{widget->ParseThickness(s); return 1;}
	if (stricmp (t, "TITLE") == 0)			{widget->ParseComponentName(s, t);	return 1;}
	if (stricmp (t, "CLOSE") == 0)			{widget->ParseComponentName(s, t);	return 1;}
	if (stricmp (t, "MINIMIZE") == 0)		{widget->ParseComponentName(s, t);	return 1;}
	if (stricmp (t, "ZOOM") == 0)				{widget->ParseComponentName(s, t);	return 1;}
	if (stricmp (t, "BUTTONWIDTH") == 0){widget->ParseButtonWidth (s);			return 1;}
	if (stricmp (t, "END") == 0)				{return 0;}
	WARNINGLOG("CFuiTheme : Unknown element type %s", t);
	return 1;
}
//==============================================================================
void CFuiTheme::Print (FILE *f)
{ /*
  fprintf (f, "THEMENAME\t%s\n", name);
  char s[8];
  TagToString (s, id);
  fprintf (f, "THEMEID\t\t%s\n", s);
  std::map<string,CFuiThemeWidget*>::iterator i;
  for (i=widgetMap.begin(); i!=widgetMap.end(); i++) {
    fprintf (f, "WIDGET\t%s\n", i->first.c_str());
    i->second->Print (f);
    fprintf (f, "\n");
  }
  fprintf (f, "\n");
	*/
}

