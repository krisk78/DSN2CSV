DSNTree is a file converter for DSN files (a standard used in France declarations to public administration).
It adds columns with parent block keys (the code of the block and the value of the wage type that is supposed to be the unique key wage type of the block).

It also adds a sequence number to divide each entity within a block (in example the block S21.G00.30 'Individual' contains several individuals). The sequence number is incremented each time the current wage type identifier is less than the previous one of the same block.

The structure of the source file is defined in the attached DSNTree.xml file. It should be reviewed for each new DSN version.
This file defines the DSN version it describes, the hierarchy of the blocks and for each one its name and its wage type key if it is not '001'.
You can maintain several DSNTree.xml files for various DSN versions and purposes (in example use the company individual identifier as key of individual block instead the national identifier).

See the inline help using /? commuter for options details.

In this version no GUI is provided but you can create .bat files to run it on files using predefined options, and without installing anything on Windows platforms.
In example, create the DSN2CSV.bat file in the folder of the executable with this content:
  @Echo OFF
  Set _this=%0
  Set _exe=%_this:DSN2CSV.bat=DSNTree.exe%
  %_exe% %* /t
  Pause
When dropping file(s) on this batch file if will sequentially process each given file, by creating a CSV file using the transpose wage types option. The CSV files are created in the folder of the source files.
You can create one bat file using particular set of options for each purpose you need.


Version history
  1.02 First production release.
