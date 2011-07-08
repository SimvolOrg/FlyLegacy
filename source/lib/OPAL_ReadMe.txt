The 4 files :
opal-ode-STLport_d.dll
opal-ode-STLport_d.lib
opal-ode-STLport.dll
opal-ode-STLport.lib

are to be placed in the downloaded OPAL third party folder :
opal-sdk-0.4.0-win32-vc8 (bin and lib sub-folders)
[http://sourceforge.net/projects/opal/files/]

They are an addition for the shipped files in "bin" and "lib" folders in order to have those libraries compliant with the STLport library used in FlyLegacy.
Please add those OPAL libraries names in the FlyLibrary project settings and add the dll ones to the proper FlyLegacy install (both release or debug)

LC : 08-01-2009
