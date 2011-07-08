USAGE :
=======

1= Stand alone
==============
Place the SaveVC8Project.exe in the main Legacy folder (where the vcproj files are situated). Launch the exe then a new folder called _compressed will be created with the cabined within (packed_1.cab).

2= Inbed VC8 tool
=================
Place the SaveVC8Project.exe in a convenient folder (sort of vc8 utilities folder)
In VC8 Tools menu bar select "external tools ..." then add the exe path to your list and make this settings :
Title = SaveProj2CAB
Command = your SaveVC8Project.exe path installation
Arguments = none
Initial Directory = $(ProjectDir) 
Close on exit = checked
others radio buttons = void

now in VC8 Tools menu list you should find "SaveProj2CAB" ... then just click.



Attention : the cab filename is Packed_0.0.0.cab and the tool add 1 to the last unit every time you save a proj.
... Packed_0.0.1.cab ... Packed_0.0.2.cab

Note : you can add some extra files to be saved creating a new vcproj file
       (i.e. My_SaveLibraries.vcproj) containing lines like :
       .../...
       RelativePath="lib\FlyLegacyPod.lib"
       RelativePath="lib\FlyLegacyUI.lib"
       .../...




~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I made this tool because with vc6 I had a very useful macro that zipped the project while with the free vc8 I can't get a similar macro (nor I have enough spare time to get deep in the controls routines)

Hope it may help you too ... 