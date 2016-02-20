# incredimail-converter-reynardware

This version of Incredimail Converter is derived from the original developed by Reynardware (and specifically David P. Owczarski) in 2009-2010. I adapted it to deal with IncrediMail 2.5's database format, which no longer uses .imm files but rather stores messages individually as .iml files. This further development is not endorsed by Reynardware. However I am very grateful for their releasing the code under a friendly license (the Mozilla Public License) thus saving me much effort!

To convert an IncrediMail 2.5 database, choose to convert either the directory `c:\users\yourusername\AppData\Local\IM\Identities\{some_uuid}\Message Store`, or explicitly pick the `MessageStore.db` file within. A directory called MessageStore will be created next to MessageStore.db which will contain the extracted Incredimail folder tree.

Click on 'Releases' above to download the initial release.

## New features above Reynardware v0.53

* Support for IncrediMail 2.5's MessageStore.db.
* Finds IncrediMail databases correctly on Windows Vista and later.
* Amends IncrediMail-style Mime/Multipart headers, which may apply to earlier database versions too.

## Caveats

* I don't have any Incredimail databases from before v2.5 to test with, so while this version should behave the same as the Reynardware version in this case, if it doesn't work it is probably worth trying their version instead.

The original author's site can be found at <https://code.google.com/archive/p/incredimail-converter-reynardware/> and his blog is at <http://reynardware.blogspot.co.uk/>

[This site](http://www.systoolsgroup.com/forensics/incredimail/) provided much useful insight into how the database is set up. It suggests that the IMM / IML file distinction specifically applies to Windows XP vs. later operating systems, so it's possible the original Reynardware converter is good for anyone stuck on XP, but for anything later you might need my version.
