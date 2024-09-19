# DSN2CSV

## Usage

DSN2CSV is a program for converting DSN files (a standard used in France for companies’ social declarations to public administration).<br>
It compiles the original file to match each block category with all its parent blocks and their key (usually the category of the block whose value is unique).

The blocks and their hierarchy, the categories and their labels are extracted from the official document [dsn-datatypes-CTXXXX.xlsx](https://www.net-entreprises.fr/declaration/norme-et-documentation-dsn/) provided by the administration.<br>
The file dsn-datatypes.cfg defines the configuration that allows this file to be used, notably adding the notion of parent missing in the ‘Header’ sheet as well as the definition of specific key categories.

The “seq” value for the key is used when there is no category whose value is unique within all records of the same block.<br>
In this case, a sequence number, rather than the value of a category, is used as the key value.<br>
The sequence number is incremented when the read category is less than or equal to the previous one of the same block.


Use the /? switch (Windows) or -h (Unix-like) to display the command help.


No graphical interface is provided in this version.<br>
However, you can use .bat files to simply launch the command from Windows by applying specific options.<br>
For example, create the DSN2CSV.bat file in the same folder as the executable file, with this content:
```bat
@Echo OFF
Set _this=%0
Set _exe=%_this:DSN2CSV.bat=dsn2csv.exe%
%_exe% %* /r:dsn-datatypes-CT2024.xlsx /l /t /v
Pause
```
Files dropped on the DSN2CSV.bat command file will be automatically converted, here with the addition of labels and in transposition mode.<br>
Files are created in the same folder as the original files.<br>
You can create multiple .bat files, calling different options, which you can then use alternately depending on your needs.

## Maintenance

It is important to update the dsn-datatypes-CTXXXX.xlsx file with each new version to ensure the correct use of DSN files of this new version.<br>
Adaptations may be required in the associated configuration file dsn-datatypes.cfg.

## Version History

| Version | Comment |
|---------|---------|
| 1.1.0	  | First production version, generally taking over the DSNTree project by adding multi-platform construction capability (Windows / Unix). |
| 1.2.0	  | Addition of labels sourced from an official file provided by the administration.<br>Clarification of the conversion code. |
