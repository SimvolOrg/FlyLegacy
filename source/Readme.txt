Fly! Legacy Source Code Readme
==============================


Development Stage : PRE-ALPHA
Last Updated      : 2005-Oct-11
Contributors      : Chris Wallace



This README file is meant for developers intending to rebuild the Fly! Legacy executable from source.  It is NOT an end-user's guide to installing, running or troubleshooting the application.

If you have problems building the source, please post the errors at the Developer forum of http://www.sourceforge.net/projects/flylegacy and I'll try to either help you through the problem or make the appropriate corrections to the makefiles.


1. SUPPORTED DEVELOPMENT ENVIRONMENTS
=====================================


1.1 Microsoft Visual C++ 6.0

The top-level source folder contains the MSVC6 Workspace and Project files for Fly! Legacy.  Both Release  and Debug builds are supported, however at this stage of the project, Release builds are compiled only very occasionally and may not be fully up to date with any build environment changes that exist in the Debug build.

To build the application in MSVC6, simply open the Workspace .dsw file, select the FlyLegacy project and the Debug build.  Open the Project Settings (Alt-F7), select the "Link" tab and change the "Output file name" so that the .exe file is written to the top-level folder of the Fly! II installation that you wish to use Fly! Legacy with.  Ensure that all dependent libraries are available (see below) and Rebuild All.


1.2 DevC++

DevC++ is a free software development environment for Windows available from Bloodshed Software at http://www.bloodshed.net/devcpp.html.  It uses the Mingw32 version of the venerable GNU Compiler.  A DevC++ project file is available in the src/ folder, named FlyLegacy.dev.  Note that there are some significant differences between the MSVC6 compile environment and Mingw32, requiring different compile settings.  These are reflected in the .dev project file, however support for DevC++ is a secondary effort and may not always be in sync with the Debug build of the MSVC6 environment.

Dev-C++ has a convenient feature to facilitate automatic installation of third party libraries.  

From the Dev-C++ menu, select "Tools->Check for Updates/Packages".  Choose a mirror site and click "Check for updates".  A list of packages available from the selected mirror will be displayed.  You may have to check several different mirror sites in order to find all of the required packages.

Alternatively, you can download the individual source code packages from the web sites listed below and build them using the supplied instructions.


1.3 GNU Automake/Autoconf

Support for GNU Automake/Autoconf is not currently available.


1.4 Others

I would like to see support for additional build environments added, so if any developers have access to any of these tools and would like to explore creating additional makefiles etc. please post to the Developers forum.
 - Visual Studio 2003 Toolkit (free compiler from Microsoft)
 - Metrowerks CodeWarrior (Mac and/or PC)
 - Open Watcom
 - etc...



2. Dependent libraries
======================

2.1 STLport

Fly! Legacy uses the STLport library for standard C++ STL templates rather than the default STL library provided with MSVC6 (Dinkumware).  STLport provides a more robust, consistent implementation and is availabe across platforms.  It can be downloaded from http://www.stlport.org

2.2 OpenGL

OpenGL 1.1 or higher must be available in your software build environment, including GLU (GL Utilities).   Note that this refers to the OpenGL headers and libraries for software development, possibly in addition to any files included with your video card driver.  For more information see http://www.opengl.org

2.3 GLUT

The GL Utility library, originally by Mark Kilgard, provides a platform-independent application layer interface to OpenGL.  There are a number of GLUT implementations now available, I recommend "FreeGLUT" available at http://freeglut.sourceforge.net

2.4 PLIB

PLIB, Steve Baker's Portable Games Library, is a collection of open-source libraries that provide a variety of useful services typically required by game applications.  Specific sub-libraries that are used at this time are: UL (Utilities), PUI (Pico-User Interface), SG (Simple Geometry), and SSG (Simple Scene Graph).  PUI will eventually be phased out in favour of the FUI (Fly! User Interface) widgets currently under development.  PLIB version 1.6.0 or higher must be available in your software build environment.  The source and links to some pre-built binaries are available from http://plib.sourceforge.net

2.5 OpenAL

Audio support for Fly! Legacy is provided via the free Open Audio Library (OpenAL) available from http://www.openal.org.  The application can be built without audio support (in which case you don't need OpenAL installed) by omitting the HAVE_OPENAL definition from the compiler options.

2.6 LibJPEG

The free JPEG image compression/decompression library is required in order to support JPEG screenshots and some debugging functions.  This should ideally be made optional by supporting a HAVE_LIBJPEG definition similar to OpenAL, however at this time the library is required.  Source is available from http://www.ijg.org


3. Downloading Source Code from CVS
===================================

In addition to the many other services SourceForge.net provides, one of the most effective for collaborative development is the hosting of a CVS repository for Fly! Legacy.  CVS, or Concurrent Version System, maintains a master copy of the source code, build files, documentation, basically anything that may require access by more than one person at a time.  Developers can each check out their own local copies of the source code, for example, make whatever additions, fixes, etc. in isolation without affecting other developers.  Then when the changes are complete and tested, they can be uploaded back into the CVS repository.  Other developers can then synchronize their local copies to the master repository.  All developers must be familiar with general CVS concepts of operation, and must have a CVS client installed on their computer in order to submit any changes for Fly! Legacy.


3.1 CVS Checkout

There are several top-level branches defined in the CVS repository for Fly! Legacy.

NOTE: When checking out any of these branches using WinCVS, please be sure that you use the "Checkout text files with the Unix linefeed (0x0a)" option in the Global Preferences, otherwise every line in text files may have an extra linefeed inserted, resulting in a file that looks like it is double-spaced.


data
	Data files that are compiled into FlyLegacy.pod.  These only need to be checked out or downloaded if changes to FlyLegacy.pod are needed.  Whenever changes are committed, an updated version of FlyLegacy.pod must be committed to the install/ branch.


docs
	Documentation in source format (typically, Microsoft Word).


install
	Files required to be installed in your Fly! II folder for Fly! Legacy to operate correctly.  These include DLLs for third-party libraries, as well as the FlyLegacy.pod data file and a default INI settings file.


source
	All of the C++ source code and build control files (MSVC workspaces/projects, etc).


web	
	Archive of the Fly! Legacy web space at http://flylegacy.sf.net.


3.2 Online Browsing

SourceForge.net also provides a web-based source code browser that makes it simple to quickly reference the latest version of any files committed to the respository, as well as any historical versions.  From the SF project page at http://www.sf.net/projects/flylegacy, scroll down to the "Browse CVS" link.


