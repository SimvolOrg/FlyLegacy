Test_SubS1_v10.dll

This is a sample subsystems.
This is also an example of how to read custom data tags from a disk file.

To install, do the following:

(1) Add the Test_SubS1_v10.dll file to the \FlyLegacy\Modules\PC directory.

(2) Modify the FLYHAWK.AMP file adding these lines to the very end of the file.

	/////
	//
	//	Test
	//
	/////

	<subs> -- --
	dpnd
	<bgno>
		<unId> -  -
		tst1
	<endo>

	////////////////////////////////
	//
	//	
	//	TST1_DLL
	//
	////////////////////////////////

	<subs> -- 
	TST1
	<bgno> -- modify key in banner options
		<unId>
		TST1
		<test>
		2
	  <mesg> -- connect to tst1 dpnd --
	  <bgno>
		  <conn>
		  tst1
	  <endo>
	<endo>




