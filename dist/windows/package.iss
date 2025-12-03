[Setup]
AppName=autodiag
AppVersion=37
DefaultDirName={pf}\autodiag
DefaultGroupName=autodiag
OutputDir=..\..\output\bin
OutputBaseFilename=autodiagSetup
Compression=lzma
SolidCompression=yes

[Files]
Source: "..\..\output\bin\autodiag.exe";               DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\output\bin\elm327sim.exe";              DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\output\bin\*.dll";                      DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\data\data\*";                    DestDir: "{app}\data\"; Flags: ignoreversion recursesubdirs
Source: "..\..\ui\*";                           DestDir: "{app}\ui\"; Flags: ignoreversion recursesubdirs
Source: "..\..\media\*";                        DestDir: "{app}\media\"; Flags: ignoreversion recursesubdirs
Source: "C:\msys64\mingw64\bin\*.dll";          DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "C:\tools\msys64\mingw64\bin\*.dll";    DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "C:\msys64\usr\bin\*.dll";              DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "C:\tools\msys64\usr\bin\*.dll";        DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{group}\autodiag"; Filename: "{app}\autodiag.exe"
