;
; Fly! Legacy Nullsoft Installer Script
;

; ----------------------------
; Include Modern UI
;

!include "MUI2.nsh"
Var FLY2FOLDER
Var BROWSE_MSG

; ----------------------------
; General Settings
;
Name			            "Fly! Legacy Alpha"
OutFile			          "FlyLegacy_Alpha_Installer.exe"
InstallDir		        "C:\FlyLegacy"
RequestExecutionLevel	none
DirText               "" "" "" $BROWSE_MSG

; ----------------------------
; Interface settings
;
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "NSIS\header.bmp"

; ----------------------------
; Pages
;
!define MUI_WELCOMEPAGE_TEXT "In order to proceed, you MUST have Fly! II (patch level 2.5.240) installed on your computer.  If you do not, please press Cancel, install Fly! II and all required patches, then re-run this installer."
!define MUI_WELCOMEFINISHPAGE_BITMAP "NSIS\welcome.bmp"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "License.txt"
!insertmacro MUI_PAGE_COMPONENTS
!define MUI_PAGE_CUSTOMFUNCTION_PRE BrowseForInstallFolder
!define MUI_DIRECTORYPAGE_TEXT_TOP "Setup will install Fly! Legacy Alpha in the following folder.  To install in a different folder, click Browse and select another folder. Click Install to start the installation. NOTE : If installing on Windows 7 or Windows Vista, please do NOT select a protected folder (e.g. C:\Program Files\FlyLegacy)"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_PAGE_HEADER_TEXT "Select Fly! II Installation Folder"
!define MUI_PAGE_HEADER_SUBTEXT ""
!define MUI_DIRECTORYPAGE_TEXT_TOP "Select the top-level folder of your Fly! II installation (this is the folder that contains Fly2.exe).  No changes will be made to any of the Fly! II files.  Files required for Fly! Legacy will be copied to a local directory within the Fly! Legacy installation."
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION "Fly! II Folder"
!define MUI_DIRECTORYPAGE_VARIABLE $FLY2FOLDER
!define MUI_PAGE_CUSTOMFUNCTION_PRE BrowseForFly2Folder
!insertmacro MUI_PAGE_DIRECTORY
Page custom CopyFly2Files
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; ----------------------------
; Use standard browse dialog message for install folder
;
Function BrowseForInstallFolder
  StrCpy $BROWSE_MSG "Select folder to install Fly! Legacy Alpha in:"
FunctionEnd

; ----------------------------
; Use custom browse dialog message for install folder
;
Function BrowseForFly2Folder
  StrCpy $BROWSE_MSG "Select Fly! II Installation Folder:"
FunctionEnd

; ----------------------------
; Languages
;
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_RESERVEFILE_LANGDLL

; ----------------------------
; Display language selection dialog
;
Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

; ----------------------------
; Installer Section
;
Section "Fly! Legacy Application"
  SetOutPath $INSTDIR

  File "FlyLegacy.exe"
  File "alut.dll"
  File "FreeImage.dll"
  File "glew32.dll"
  File "opal-ode.dll"
  File "opal-ode_d.dll"
  File "pthreadVC2.dll"
  File "License.txt"
  File "FlyLegacy_Readme.txt"
  File /r /x "CVS" "Aircraft"
  File /r /x "CVS" "Charts"
  File /r /x "CVS" "Clouds"
  File /r /x "CVS" "Data"
  File /r /x "CVS" "Debug"
  File /r /x "CVS" "Doc"
  File /r /x "CVS" "FlightPlan"
  File /r /x "CVS" "Logs"
  File /r /x "CVS" "Metar"
  File /r /x "CVS" "Modules"
  File /r /x "CVS" "Runways"
  File /r /x "CVS" "Saved Simulations"
  File /r /x "CVS" "SDK"
  File /r /x "CVS" "System"
  File /r /x "CVS" "Ui"

  ; Write uninstall information to registry  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FlyLegacy" "DisplayName" "Fly! Legacy"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FlyLegacy" "UninstallString" "$INSTDIR\FlyLegacy_Alpha_Uninstall.exe"
  
  ; Generate uninstaller application
  WriteUninstaller $INSTDIR\FlyLegacy_Alpha_Uninstall.exe
SectionEnd

; ----------------------------
; Start Menu Section
;
Section "Start Menu Shortcut"
  CreateDirectory "$SMPROGRAMS\Fly Legacy Alpha"
  CreateShortCut "$SMPROGRAMS\Fly Legacy Alpha\Fly! Legacy.lnk" "$INSTDIR\FlyLegacy.exe"
  CreateShortCut "$SMPROGRAMS\Fly Legacy Alpha\Uninstall Fly! Legacy.lnk" "$INSTDIR\FlyLegacy_Alpha_Uninstall.exe"
SectionEnd

; ----------------------------
; Desktop Shortcut Section
;
Section "Desktop Shortcut"
  CreateShortCut "$DESKTOP\Fly! Legacy.lnk" "$INSTDIR\FlyLegacy.exe"
SectionEnd

; ----------------------------
; Quick launch Section
;
Section "Quick Launch Shortcut"
  CreateShortCut "$QUICKLAUNCH\Fly! Legacy.lnk" "$INSTDIR\FlyLegacy.exe"
SectionEnd

;-----------------------------
; Install Visual C++ redistributable
;
Section -InstallVCRedist
  SetOutPath "$TEMP"
  File /oname=vs2008_vcredist_x86.exe "redist\vs2008_vcredist_x86.exe"
  DetailPrint "Running Microsoft Visual C++ 2008 Redistributable Setup..."
  ExecWait "$TEMP\vs2008_vcredist_x86.exe"
  DetailPrint "Finished Microsoft Visual C++ 2008 Redistributable Setup"
  Delete "$TEMP\vs2008_vcredist_x86.exe"
  SetOutPath "$INSTDIR"
SectionEnd

;-----------------------------
; Install OpenAL
;
Section -InstallOpenAL
  ; Check for presence of OpenAL dll in system folder
  IfFileExists "$SYSDIR\OpenAL32.dll" ExitOpenAL InstOpenAL
  
InstOpenAL:
  SetOutPath "$TEMP"
  File /oname=oalinst.exe "redist\oalinst.exe"
  DetailPrint "Running OpenAL Setup..."
  ExecWait "$TEMP\oalinst.exe"
  DetailPrint "Finished OpenAL Setup"
  Delete "$TEMP\oalinst.exe"
  SetOutPath "$INSTDIR"
  
ExitOpenAL:
SectionEnd

;-----------------------------
; Install Python
;
Section -InstallPython
  ; Check for presence of Pythonxx.dll in system folder
  IfFileExists "$SYSDIR\Python26.dll" ExitPython InstPython
  
InstPython:
  SetOutPath "$TEMP"
  File /oname=python-2.6.5.msi "redist\python-2.6.5.msi"
  DetailPrint "Running Python 2.6 Setup..."
  File /oname=python.bat "NSIS\python.bat"
  ExecWait "$TEMP\python.bat"
  DetailPrint "Finished Python 2.6 Setup"
  Delete "$TEMP\python-2.6.5.msi"
  Delete "$TEMP\python.bat"
  SetOutPath "$INSTDIR"
  
ExitPython:
SectionEnd

; ----------------------------
; Custom page to copy Fly! II files to Fly! Legacy installation
;
Function CopyFly2Files
  ; Create local Fly2Files folder and copy required files
  CreateDirectory $INSTDIR\Fly2Files
  CreateDirectory $INSTDIR\Fly2Files\Aircraft
  CopyFiles       $FLY2FOLDER\Aircraft\ACCOMMON.POD $INSTDIR\Fly2Files\Aircraft
  CopyFiles       $FLY2FOLDER\Aircraft\FLYHAWK.POD  $INSTDIR\Fly2Files\Aircraft
  CreateDirectory $INSTDIR\Fly2Files\Scenery
  CreateDirectory $INSTDIR\Fly2Files\Scenery\Shared
  CopyFiles       $FLY2FOLDER\Scenery\Shared\generic.pod    $INSTDIR\Fly2Files\Scenery\Shared
  CopyFiles       $FLY2FOLDER\Scenery\Shared\cities.pod     $INSTDIR\Fly2Files\Scenery\Shared
  CopyFiles       $FLY2FOLDER\Scenery\Shared\trucks.pod     $INSTDIR\Fly2Files\Scenery\Shared
  CreateDirectory $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\AIRPORT.POD    $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\ATSGRAPH.POD   $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\ATSROUTE.POD   $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\camera.sav     $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\CENTERDB.POD   $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\COAST.POD      $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\COMM.POD       $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\EDITOR.POD     $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\GENTEX.POD     $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\GEOGRAPH.POD   $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\GLOBE.POD      $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\ILS.POD        $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\METAR.POD      $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\NAVAID.POD     $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\OBSTRUCT.POD   $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\PATCH.POD      $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\RUNWAY.POD     $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\STARTUP.POD    $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\TAXIWAYS.POD   $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\UI.POD         $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\UIMAPS.POD     $INSTDIR\Fly2Files\System
  CopyFiles       $FLY2FOLDER\System\WAYPOINT.POD   $INSTDIR\Fly2Files\System
  CreateDirectory $INSTDIR\Fly2Files\Taxiways
  CopyFiles       $FLY2FOLDER\Taxiways\*.POD        $INSTDIR\Fly2Files\Taxiways
  
  ; Update FlyLegacy.ini to refer to local Fly2Files folder
  WriteINIStr $INSTDIR\System\FlyLegacy.ini "UI" "flyRootFolder" "$INSTDIR\Fly2Files"
FunctionEnd

; ----------------------------
; Uninstaller Section
;
Section "un.Uninstaller Section"
  RMDir /r $INSTDIR\Fly2Files
  RMDir /r $INSTDIR\Aircraft
  RMDir /r $INSTDIR\Charts
  RMDir /r $INSTDIR\Clouds
  RMDir /r $INSTDIR\Data
  RMDir /r $INSTDIR\Debug
  RMDir /r $INSTDIR\Doc
  RMDir /r $INSTDIR\FlightPlan
  RMDir /r $INSTDIR\Logs
  RMDir /r $INSTDIR\Metar
  RMDir /r $INSTDIR\Modules
  RMDir /r $INSTDIR\Runways
  RMDir /r "$INSTDIR\Saved Simulations"
  RMDir /r $INSTDIR\SDK
  RMDir /r $INSTDIR\System
  RMDir /r $INSTDIR\Ui
  Delete "$INSTDIR\*.dll"
  Delete $INSTDIR\License.txt
  Delete $INSTDIR\FlyLegacy_Readme.txt
  Delete "$INSTDIR\__DDEBUG*.*"
  Delete $INSTDIR\FlyLegacy.exe
  Delete $INSTDIR\FlyLegacy_Alpha_Uninstall.exe
  RMDir $INSTDIR

  ; Delete desktop shortcut
  Delete "$DESKTOP\Fly! Legacy.lnk"
  
  ; Delete Start Menu Shortcuts
  Delete "$SMPROGRAMS\Fly Legacy Alpha\Fly! Legacy.lnk"
  Delete "$SMPROGRAMS\Fly Legacy Alpha\Uninstall Fly! Legacy.lnk"
  RmDir  "$SMPROGRAMS\Fly Legacy Alpha"

  ; Delete Quick Launch shortcut
  Delete "$QUICKLAUNCH\Fly! Legacy.lnk"
  
  ;Delete Uninstaller And Unistall Registry Entries
  DeleteRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\FlyLegacy"
SectionEnd

