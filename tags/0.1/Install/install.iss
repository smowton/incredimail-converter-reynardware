[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{CDD74C77-BCAC-4545-A52B-FC1F575B9900}
AppName=Reynardware Incredimail Converter
AppVerName=Reynardware Incredimail Converter 0.1
AppPublisher=Reynardware
DefaultDirName={pf}\Reynardware Incredimail Converter
DefaultGroupName=Reynardware Incredimail Converter
AllowNoIcons=yes
LicenseFile=.\Mozilla Public License.txt
InfoAfterFile=.\Readme.txt
OutputDir=..
OutputBaseFilename=Setup
Compression=lzma/max
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\Build\Release\Reynard Incredimail Converter.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "readme.txt"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Reynardware Incredimail Converter"; Filename: "{app}\Reynard Incredimail Converter.exe"
Name: "{group}\{cm:UninstallProgram,Reynardware Incredimail Converter}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Reynardware Incredimail Converter"; Filename: "{app}\Reynard Incredimail Converter.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\Reynard Incredimail Converter.exe"; Description: "{cm:LaunchProgram,Reynardware Incredimail Converter}"; Flags: nowait postinstall skipifsilent

