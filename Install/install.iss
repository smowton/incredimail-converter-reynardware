[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{CDD74C77-BCAC-4545-A52B-FC1F575B9900}
AppName=Incredimail Converter
AppVerName=Incredimail Converter CS Release 2
AppPublisher=Chris Smowton
DefaultDirName={pf}\Incredimail Converter
DefaultGroupName=Incredimail Converter
AllowNoIcons=yes
LicenseFile=.\Mozilla Public License.txt
AppPublisherURL=https://github.com/smowton/incredimail-converter-reynardware
AppSupportURL=https://github.com/smowton/incredimail-converter-reynardware
AppUpdatesURL=https://github.com/smowton/incredimail-converter-reynardware
OutputDir=..
OutputBaseFilename=IncredimailConverter-CSr2-Setup
Compression=lzma/max
PrivilegesRequired=none
SolidCompression=yes
VersionInfoVersion=0.60

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\Source\Release\Incredimail Converter.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "Users Guide.pdf"; DestDir: "{app}"; Flags: ignoreversion
Source: "Mozilla Public License.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "gpl-3.0.txt"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Incredimail Converter"; Filename: "{app}\Incredimail Converter.exe"
Name: "{group}\Users Guide"; Filename: "{app}\Users Guide.pdf"
Name: "{group}\GPL3.0.txt"; Filename: "{app}\gpl-3.0.txt"
Name: "{group}\Mozilla Public License.txt"; Filename: "{app}\Mozilla Public License.txt"
Name: "{group}\Website"; Filename: "https://github.com/smowton/incredimail-converter-reynardware"
Name: "{group}\Reynardware Website"; Filename: "http://code.google.com/p/incredimail-converter-reynardware/"
Name: "{group}\{cm:UninstallProgram,Incredimail Converter}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Incredimail Converter"; Filename: "{app}\Incredimail Converter.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\Incredimail Converter.exe"; Description: "{cm:LaunchProgram,Incredimail Converter}"; Flags: nowait postinstall skipifsilent

