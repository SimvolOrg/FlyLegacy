Rotw  Yet Another Google Earth Tool for FLYLegacy (v0.9)
FOR PC ONLY

ROTW October 2009(c) 
(http://fly.simvol.org/)

author = laurentC	
(http://...)
email contact : baloon@hotmail.com 


***************************************************************************************************
 This little plugin for FLYLegacy allows to create a direct link between a Google Earth window and
 an instance of FlyLegacy to follow your position on the GoogleEarth view.
 In a LAN you can point with Google Earth to the FlyLegacy kml files directly.

***************************************************************************************************


***************************************************************************************************
Interesting links :
- http://www.wikihow.com/Overlay-Sectional-Aeronautical-Charts-in-Google-Earth

***************************************************************************************************
History :
- 0.9  first public release

***************************************************************************************************
Installation :
[Ensure you have the most up-to-date version of Google Earth by selecting Check for Updates Online
 in the Help menu]


- install :
           Legacy_google_earth_Vxx.dll in ...\FlyLegacy\modules\PC (create this folder if not present)
           
- create a new folder :
           "GoogleEarth" in the FlyLegacy main root (...\FlyLegacy\GoogleEarth)
           and put test_earth_link.kml in it
					 
- place :
           ROTW_YAGET_V10_.WIN in	...\FlyLegacy\UI\templates folder		

***************************************************************************************************

***************************************************************************************************
Usage :
=======
1- open FlyLegacy (usual operations)
2- put FlyLegacy in the backstage if you are in autoFullScreen and you have a sole monitor (TAB key)
3- call the menu bar (space key) and select "Plugins\YAGET" once
4- the "Google Earth Interface" window should appear now
5- push the "Loop" button - then the "Track Pos" button : 
  a "test_track_path.kml" should be create in the GoogleEarth FL folder 
6- double click on the "test_earth_link.kml" file in the FlyLegacy GoogleEarth folder (or launch 
  Google Earth and open this file from it)
7- from now on you should be watching your acual position on the Google window ...
 (adjust Google Earth in Tools/Options/Control/FlyToTour Settings from "slow" to "fast" for a better display)

Note that with latest GoogleEarth versions [v.5.1 and beyond] instead of step [6-] you need to "Add a network link" in "temporary places" then adjust the "refresh" tab checking the Fly To View radio button and setting a few sec. in the Time Based Refresh section.

Remove the "Google Earth Interface" but keep tracking :
=======================================================
- select "Plugins\YAGET" once more in the menu bar (space key)
- select "Plugins\YAGET" once more in the menu bar (space key) to make the window appear again

Stop Tracking :
===============
- in the "Google Earth Interface" window push "Loop" or the "Tracking.." button
  Notice that Google earth stops tracking your position and you need to refresh the link
	(right mouse button upon "test_earth_link.klm\Link"
	
Close the window :
================== 
- in the "Google Earth Interface" window push the upper right crossed red close button

***************************************************************************************************
NB: if you have a home LAN or two PC connected you can lauch GoogleEarth from one PC and open the KML
    files of the other PC in order to have Fly!Legacy on a monitor and GoogleEarth on the second

***************************************************************************************************
KNOWN ISSUES :

- 
