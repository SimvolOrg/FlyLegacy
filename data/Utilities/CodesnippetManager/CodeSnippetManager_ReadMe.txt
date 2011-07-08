Code snippet manager
--------------------
Used to add code snippets in the current vc8 file

Usage
-----

  batch command : CodeSnippetManager.exe database_filepath(%s) filename(%s) line(%d)
                  
                  database_filepath = complete path of database (string)
                  filename          = file name to modify (in the same folder as copyfile.exe !!!) (string) 
                  line              = line num where to add (int)

             ex : CodeSnippetManager .\DataBase.dat main.cpp 2

VC8 install
-----------

  in vc8 external tools add a new item with this settings :
	  title             :  as you wish
	  command           :  K:\Program Files\Microsoft Visual Studio\C++TOOLS\VC8_TOOLS\CodeSnippetManager.exe
        arguments         : "K:\Program Files\Microsoft Visual Studio\C++TOOLS\VC8_TOOLS\DataBase.xml" $(ItemFileName)$(ItemExt) $(CurLine)
        initial directory : $(ItemDir)


Infos
-----
  database file : database.dat

  ex :

[DATA]
TITLE = AddLines DB

[DATA000]
NAME=OutFile Debug
LINES = <<<MULTI
//
#ifdef _DEBUG	
	FILE *fp_debug;
	if(!(fp_debug = fopen("__DDEBUG_.txt", "a")) == NULL)
	{
		int test = 0;
		fprintf(fp_debug, "%d\n", test);
		fclose(fp_debug); 
	}
#endif
MULTI

[DATA001]
NAME=APIDrawNoticeToUser
LINES= <<<MULTI
#ifdef _DEBUG	
	///////////////// remove later ////////////////////////////////
	char buffer[128] = {0};
	sprintf(buffer, "%d", 0);
	APIDrawNoticeToUser(buffer,1);
	///////////////// remove later ////////////////////////////////
#endif
MULTI

.../...

[DATA006]
NAME = Copy & Paste
LINES =  <<<MULTI
Place holder
MULTI

[DATA007]
NAME = Copy & Paste
LINES =  <<<MULTI
Place holder
MULTI

[DATA999]
; end 