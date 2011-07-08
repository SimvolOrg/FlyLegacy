Code snippet manager
--------------------
Used to add code snippets in the current vc8 file

Usage
-----

  batch command : copyfile.exe database_filepath(%s) filename(%s) line(%d)
                  
                  database_filepath = complete path of database (string)
                  filename          = file name to modify (in the same folder as copyfile.exe !!!) (string) 
                  line              = line num where to add (int)

             ex : copyfile .\DataBase.xml main.cpp 2

VC8 install
-----------

  in vc8 external tools add a new item with this settings :
              arguments         : "K:\Program Files\Microsoft Visual Studio\C++TOOLS\VC8_TOOLS\DataBase.xml" $(ItemFileName)$(ItemExt) $(CurLine)
              initial directory : $(ItemDir)

First argument is the database complete path
Second argument is the current file
Third argument is the line wher the snippet is added

Infos
-----
  database file : database.xml

  ex :

      <DATA>

      <TITLE>AddLines DB</TITLE>

      <DATA000>
      <NAME>Istanbul1</NAME>
      <LINES>
      Test1
      </LINES>
      </DATA000>

      <DATA001>
      <NAME>NewYork2</NAME>
      <LINES>Test 123456789
      </LINES>
      </DATA001>

      </DATA>