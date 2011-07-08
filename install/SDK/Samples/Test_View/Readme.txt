Test_View1_v10.dll

This is a sample for an additional camera.


To install, do the following:

(1) Add the Test_View1_v10.dll file to the \FlyLegacy\Modules\PC directory.

(2) Modify the DATA\CAMERAS_LEGACY.TXT file adding a new line for this new camera.

7
spot,Spot Camera,1
orbt,Orbit Camera,0
obsr,Stationary Camera,0
flyb,Fly-By Camera,0
towr,Tower Camera,0
VIEW,Test DLL Camera,1
over,Over Camera,1

Shifting camera you shoul see spot, VIEW and over cameras (the others ones are disabled with the "0" at the end of the line.




