== ReynardWare Incredimail Converter - v0.1 04/06/09==

=== Background ===
My wife was recently using Incredimail email client and I finally convinced her to use Thunderbird. (Now she loves Thunderbird!) I recently found out that there was no way to convert her Incredimail folders to Thunderbird. So, I looked on the web and found a couple of converters which would convert to .eml (then to Thunderbird), but some of them did not work or had to pay money for them. Yuk!

To solve my problem, I spend some time writing this small converter application which takes the latest format of the Incredimail database (v4 and v5) and converts all the emails to .eml.  This is a freeware application and I am also willing to support it as much as I can.  Unfortunately, I don't have a lot of time so the updates and features will be slow.

=== Brief User Instructions ===
Here is a brief user guide on using the converter.

The first edit box, the user should enter in the Incredimail database file, which is the .imm file.  The application will try to open the associated Incredimail header file, the .imh file, in the same directory.  Once you selected a database file, the email count to the right should update.  The second edit box, the user should specify where all the attachments are located.

After the database and attachment directories are selected, the user should click the 'Convert' button.  The progress bar will update and will identify on what email the converter is being converted.  In the same directory where the database file is located, a export directory will be created, with the same name of the database.

A couple of special notes:
(1)  A temporary directory called Convert_Temp will be created in root of the C drive.  This is only a temporary directory and this will be removed in future versions.
(2)  This application runs in a single thread, so if you click anything else it will seem the application hung.  Even though it seems like it hung, it is still running in the background.  If this happen, once the converter is done converting the current database, it will return to normal.

=== Revision ==
--- v0.1  2009-04-06
Initial Version!  Just want to get the source and project built so the public can start using the converter.

=== TODO LIST ===
+ Check and put up increadimail header information in dialog box
+ Clean up the header files
+ Get rid of the C:\Convert_Temp  (create a delete directory routine)
+ Rewrite the get_email_offset_and_size() function
+ Massively clean up the WinLoad() function.
+ Fix the about dialog box
+ Rewrite the encode() function to use Win32 API calls
+ Make a secondary thread for the processing of the emails
+ Write a user manual
+ Make an nice icon (fox like)
+ Create an output directory for the export

=== Known Bugs ===
+ If the attachment directory and database directory parameters are NULL, application will crash.

=== Limitations ===
+ Only works with v4 and v5 of the incredimail database