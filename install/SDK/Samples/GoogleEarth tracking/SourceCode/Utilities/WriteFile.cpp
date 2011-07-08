// WriteFileBase.cpp: implementation of the CWriteFileBase class.
//
//////////////////////////////////////////////////////////////////////

#include "WriteFile.h"

using namespace my_file;

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

void CDataString::TraverseDataAndPrint (FILE *fp)
{
  for (it_ = data_string_.begin (); it_ != data_string_.end (); ++it_) {
	//
    fprintf(fp, "%s\n", it_->c_str ());
  }
}

void CDataString::AddString (std::string s)
{
  data_string_.push_back (s);
}

//////////////////////////////////////////////////////////////////////
// CWriteFileBase
//////////////////////////////////////////////////////////////////////

CWriteFileBase::CWriteFileBase (std::string filename)
 : fp_ (NULL)
{
  data_      = new CDataString ();

  #ifdef _DEBUG	
  if (data_) {
	  FILE *fp_debug;
	  if(!(fp_debug = fopen("__DDEBUG_legacy_google2.txt", "a")) == NULL)
	  {
		  int test = 0;
		  fprintf(fp_debug, "CWriteFileBase::CWriteFileBase : constructor data_%d\n", data_);
		  fclose(fp_debug); 
	  }
  }
  #endif

  filename_  = filename + ".kml";
  file_flag_ = true;
  OpenFile ();
}

CWriteFileBase::~CWriteFileBase (void)
{
	if(fp_)
	{
		fclose(fp_);
        #ifdef _DEBUG	
	        FILE *fp_debug;
	        if(!(fp_debug = fopen("__DDEBUG_legacy_google2.txt", "a")) == NULL)
	        {
		        int test = 0;
		        fprintf(fp_debug, "CWriteFileBase::~CWriteFileBase fclose %d\n", fp_);
		        fclose(fp_debug); 
	        }
        #endif
        fp_ = NULL;
	}
    if (data_)
    {
        delete (data_);
        #ifdef _DEBUG	
	        FILE *fp_debug;
	        if(!(fp_debug = fopen("__DDEBUG_legacy_google2.txt", "a")) == NULL)
	        {
		        int test = 0;
		        fprintf(fp_debug, "CWriteFileBase::~CWriteFileBase delete data_%d\n", data_);
		        fclose(fp_debug); 
	        }
        #endif
        data_ = NULL;
    }
    #ifdef _DEBUG	
	    FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_legacy_google2.txt", "a")) == NULL)
	    {
		    int test = 0;
		    fprintf(fp_debug, "CWriteFileBase::~CWriteFileBase %d\n", fp_);
		    fclose(fp_debug); 
	    }
    #endif
}

void CWriteFileBase::OpenFile (void)
{
  if (file_flag_) {
	if(!(fp_ = fopen(filename_.c_str (), "w")) == NULL)
	{
//	  WriteFile (); // test
        #ifdef _DEBUG	
	        FILE *fp_debug;
	        if(!(fp_debug = fopen("__DDEBUG_legacy_google2.txt", "a")) == NULL)
	        {
		        int test = 0;
		        fprintf(fp_debug, "CWriteFileBase::OpenFile %s %d\n", filename_.c_str (), fp_);
		        fclose(fp_debug); 
	        }
        #endif
	}
    else {
      file_flag_ = false;
      fp_ = NULL;
    }
  }
}

void CWriteFileBase::WriteFile (void) const
{
  if (file_flag_) {
    data_->AddString ("test 1");
    data_->AddString ("test 2");
	data_->TraverseDataAndPrint (fp_);
  }
}

void CWriteFileBase::Write (void) const
{
    WriteFile ();
    #ifdef _DEBUG	
	    FILE *fp_debug;
	    if(!(fp_debug = fopen("__DDEBUG_legacy_google2.txt", "a")) == NULL)
	    {
		    int test = 0;
		    fprintf(fp_debug, "CWriteFileBase::Write %d\n", fp_);
		    fclose(fp_debug); 
	    }
    #endif
}

void CWriteFileBase::AddCoordinates (const SUserPos &pos) const
{
  if (file_flag_) {
    char buffer[128];
    //
    sprintf (buffer,  "      %.5f,%.5f,%.0f", pos.lon, pos.lat, pos.ga);
    data_->AddString (buffer);
  }
}

//////////////////////////////////////////////////////////////////////
// CWritePosition
//////////////////////////////////////////////////////////////////////

void CWritePosition::WriteFile (void) const
{
  if (file_flag_) {

    char buffer[128];

    data_->AddString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    data_->AddString ("<kml xmlns=\"http://earth.google.com/kml/2.0\">");
    data_->AddString ("  <Placemark>");
    data_->AddString ("    <name>Aircraft</name>");
    data_->AddString ("    <description>Our Aircraft Position");
    sprintf (buffer,  "    Alt : %.1f ft</description>", pos_.alt);
    data_->AddString (buffer);
    data_->AddString ("    <visibility>1</visibility>");
    data_->AddString ("    <styleUrl>root://styles#default+icon=0x307</styleUrl>");
    data_->AddString ("    <Style>");
    data_->AddString ("	     <labelColor>ff00ffff</labelColor>");
    data_->AddString ("      <IconStyle>");
    data_->AddString ("        <Icon>");
    data_->AddString ("          <href>root://icons/palette-2.png</href>");
    data_->AddString ("          <y>32</y>");
    data_->AddString ("          <w>32</w>");
    data_->AddString ("          <h>32</h>");
    data_->AddString ("        </Icon>");
    data_->AddString ("      </IconStyle>");
    data_->AddString ("    </Style>");
    data_->AddString ("    <Point>");
    data_->AddString ("      <extrude>1</extrude>");
    data_->AddString ("      <altitudeMode>relativeToGround</altitudeMode>");
    //                       <coordinates>-122.1664,37.49344,294.8975</coordinates>
    sprintf (buffer,  "      <coordinates>%.5f,%.5f,%.0f</coordinates>", pos_.lon, pos_.lat, pos_.ga);
    data_->AddString (buffer);
    data_->AddString ("    </Point>");
    data_->AddString ("  </Placemark>");
    data_->AddString ("</kml>");

	data_->TraverseDataAndPrint (fp_);
  }
}

//////////////////////////////////////////////////////////////////////
// CWritePath
//////////////////////////////////////////////////////////////////////

CWritePath::CWritePath (std::string s)
 : CWriteFileBase (s)
{
 Initialise ();
}

void CWritePath::Initialise (void) const
{
  if (file_flag_) {
    data_->AddString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    data_->AddString ("<kml xmlns=\"http://earth.google.com/kml/2.0\">");
    data_->AddString ("<Document>");
    data_->AddString ("  <name>User Path</name>");
    data_->AddString ("  <Style id=\"khStyle527\">");
    data_->AddString ("    <IconStyle id=\"khIconStyle529\">");
    data_->AddString ("    </IconStyle>");
    data_->AddString ("    <LabelStyle id=\"khLabelStyle531\">");
    data_->AddString ("    </LabelStyle>");
    data_->AddString ("    <LineStyle id=\"khLineStyle532\">");
    data_->AddString ("      <color>ff00ff55</color>");
    data_->AddString ("    </LineStyle>");
    data_->AddString ("    <PolyStyle id=\"khPolyStyle533\">");
    data_->AddString ("      <color>85ffffff</color>");
    data_->AddString ("      <fill>0</fill>");
    data_->AddString ("    </PolyStyle>");
    data_->AddString ("  </Style>");
    data_->AddString ("  <Placemark>");
    data_->AddString ("    <name>FS - Flight Path</name>");
    data_->AddString ("    <description>Flight Simulator Path");
    data_->AddString ("    rotw 2006 (c)");
    data_->AddString ("    </description>");
    data_->AddString ("    <styleUrl>#khStyle527</styleUrl>");
    data_->AddString ("    <Style>");
    data_->AddString ("      <PolyStyle>");
    data_->AddString ("        <color>4c00ff55</color>");
    data_->AddString ("        <fill>1</fill>");
    data_->AddString ("      </PolyStyle>");
    data_->AddString ("    </Style>");
    data_->AddString ("    <LineString>");
    data_->AddString ("      <extrude>1</extrude>");
    data_->AddString ("      <altitudeMode>absolute</altitudeMode>");
    data_->AddString ("      <coordinates>");
  }
}

void CWritePath::WriteFile (void) const
{
  if (file_flag_) {
    //
    data_->AddString ("      </coordinates>");
    data_->AddString ("    </LineString>");
    data_->AddString ("  </Placemark>");
    data_->AddString ("</Document>");
    data_->AddString ("</kml>");
//
	data_->TraverseDataAndPrint (fp_);
  }
}

//////////////////////////////////////////////////////////////////////
// CWriteTrackPath
//////////////////////////////////////////////////////////////////////

void CWriteTrackPath::WriteFile (void) const
{
  if (file_flag_) {
    char buffer[128];

    data_->AddString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    data_->AddString ("<kml xmlns=\"http://earth.google.com/kml/2.0\">");
    data_->AddString (" <Document>");
    data_->AddString ("  <Placemark>");
    data_->AddString ("    <name>Aircraft</name>");
    data_->AddString ("    <Style>");
    data_->AddString ("     <IconStyle>");
    data_->AddString ("      <Icon>");
    data_->AddString ("       <href>root://icons/palette-3.png</href>");
    data_->AddString ("       <x>128</x>");
    data_->AddString ("       <y>32</y>");
    data_->AddString ("       <w>32</w>");
    data_->AddString ("       <h>32</h>");
    data_->AddString ("      </Icon>");
    data_->AddString ("     </IconStyle>");
    data_->AddString ("    </Style>");
    data_->AddString ("    <Point>");

    sprintf (buffer,  "     <coordinates>%010.5f,%010.5f,0.0</coordinates>", pos_.lon, pos_.lat);
    data_->AddString (buffer);

    data_->AddString ("    </Point>");
    data_->AddString ("  </Placemark>");
    data_->AddString ("  <LookAt>");
    sprintf (buffer,  "   <longitude>%010.5f</longitude>", pos_.lon);
    data_->AddString (buffer);
    sprintf (buffer,  "   <latitude>%010.5f</latitude>", pos_.lat);
    data_->AddString (buffer);
    sprintf (buffer,  "   <heading>%03d.00</heading>", pos_.head);
    data_->AddString (buffer);
    data_->AddString ("   <tilt>45.00</tilt>");

    data_->AddString ("   <range>15000.00</range>");
    data_->AddString ("   <altitudeMode>clampToGround</altitudeMode>");

    data_->AddString ("  </LookAt>");
    data_->AddString ("</Document>");
    data_->AddString ("</kml>");

	data_->TraverseDataAndPrint (fp_);
  }
}
