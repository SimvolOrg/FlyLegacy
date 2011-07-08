Test_Gauge_v10.dll

This is a sample gauge that uses some drawing primitives, as well as communicates with an existing Fly! system (the altimeter) to provide visual feedback.  This is also an example of how to read custom data tags from a disk file.

To install, do the following:

(1) Add the Test_Gauge_v10.dll file to the \FlyLegacy\Modules\PC directory.

(2) Add VOR.ACT & VOR.PBM in the ART FlyLegacy folder

(3) Modify the flyhawkfp.PNL file adding these lines to the very end of the file.

////////////////////////////////////////////////
//
//	Sample Gauge
//
////////////////////////////////////////////////

<gage> ==== GAUGE ENTRY ====
S__G
<bgno> ==== BEGIN GAUGE ENTRY ====
	<unid> ---- unique id ----
	S__G
	<size> ---- x,y,xsize,ysize ----
	197
	170
	90
	90
	<test>
	15
	<mesg> -- connect to altimeter Subsystem --
	<bgno>
	<mesg>
	<bgno>
		<conn>
		alti
		<dtag>
		alti
	<endo>
<endo>

When you start FlyLegacy and the FlyHawk (cessna 172R), this new gauge will appear in place of the altimeter gauge.



