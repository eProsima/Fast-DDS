!include x64.nsh

Var X64VS2010Needed
Var I86VS2010Needed
Var X64VS2013Needed
Var I86VS2013Needed

Function InstallRedistributables

    StrCpy $X64VS2010Needed "1"
    StrCpy $I86VS2010Needed "1"
    StrCpy $X64VS2013Needed "1"
    StrCpy $I86VS2013Needed "1"

    # Check if it is necessary to install to x64VS2010
    ${If} ${RunningX64}
        ${If} ${SectionIsSelected} ${libraries_x64Win64VS2010}
        ${OrIf} ${SectionIsSelected} ${libraries_i86Win32VS2010}
	    ClearErrors
            SetRegView 64
            ReadRegDword $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{1D8E6291-B0D5-35EC-8441-6616F567A0F7}" "Version"
            IfErrors 0 VC2010RedistInstalled
            StrCpy $X64VS2010Needed "0"
        ${EndIf}
    ${Else}
        # Check if it is necessary to install to i86VS2010
        ${If} ${SectionIsSelected} ${libraries_i86Win32VS2010}
	    ClearErrors
            ReadRegDword $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{F0C3E5D1-1ADE-321E-8167-68EF0DE699A5}" "Version"
            IfErrors 0 VC2010RedistInstalled
            StrCpy $I86VS2010Needed "0"
        ${EndIf}
    ${EndIf}

    VC2010RedistInstalled:

    # Check if it is necessary to install to x64VS2013
    ${If} ${RunningX64}
        ${If} ${SectionIsSelected} ${libraries_x64Win64VS2013}
        ${OrIf} ${SectionIsSelected} ${libraries_i86Win32VS2013}
	    ClearErrors
            SetRegView 64
            ReadRegDword $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A749D8E6-B613-3BE3-8F5F-045C84EBA29B}" "Version"
            IfErrors 0 VC2013RedistInstalled
            StrCpy $X64VS2013Needed "0"
        ${EndIf}
    ${Else}
        # Check if it is necessary to install to i86VS2013
        ${If} ${SectionIsSelected} ${libraries_i86Win32VS2013}
	    ClearErrors
            ReadRegDword $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{13A4EE12-23EA-3371-91EE-EFB36DDFFF3E}" "Version"
            IfErrors 0 VC2013RedistInstalled
            StrCpy $I86VS2013Needed "0"
        ${EndIf}
    ${EndIf}

    VC2013RedistInstalled:

    ${If} ${RunningX64}
        StrCmp $X64VS2010Needed "1" notinstall2010 install2010
    ${Else}
        StrCmp $I86VS2010Needed "1" notinstall2010 install2010
    ${EndIf}

    install2010:
    messageBox MB_YESNO|MB_ICONQUESTION "$(^Name) needs Visual Studio 2010 Redistributable packages.$\nDo you want to download and install them?" IDNO notinstall2010

    ${If} ${RunningX64}
        NSISdl::download http://download.microsoft.com/download/A/8/0/A80747C3-41BD-45DF-B505-E9710D2744E0/vcredist_x64.exe $TEMP\vcredist_x64.exe
        Pop $R0 ; Get the return value
        StrCmp $R0 "success" 0 +3
        ExecWait "$TEMP\vcredist_x64.exe"
        Goto +2
        MessageBox MB_OK "vcredist_x64.exe download failed: $R0"
    ${Else}
        NSISdl::download http://download.microsoft.com/download/C/6/D/C6D0FD4E-9E53-4897-9B91-836EBA2AACD3/vcredist_x86.exe $TEMP\vcredist_x86.exe
        Pop $R0 ; Get the return value
        StrCmp $R0 "success" 0 +3
        ExecWait "$TEMP\vcredist_x86.exe"
        Goto +2
        MessageBox MB_OK "vcredist_x86.exe download failed: $R0"
    ${EndIf}

    notinstall2010:

    ${If} ${RunningX64}
        StrCmp $X64VS2013Needed "1" notinstall2013 install2013
    ${Else}
        StrCmp $I86VS2013Needed "1" notinstall2013 install2013
    ${EndIf}

    install2013:
    messageBox MB_YESNO|MB_ICONQUESTION "$(^Name) needs Visual Studio 2013 Redistributable packages.$\nDo you want to download and install them?" IDNO notinstall2013

    ${If} ${RunningX64}
        NSISdl::download http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x64.exe $TEMP\vcredist_x64.exe
        Pop $R0 ; Get the return value
        StrCmp $R0 "success" 0 +3
        ExecWait "$TEMP\vcredist_x64.exe"
        Goto +2
        MessageBox MB_OK "vcredist_x64.exe download failed: $R0"
    ${Else}
        NSISdl::download http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x86.exe $TEMP\vcredist_x86.exe
        Pop $R0 ; Get the return value
        StrCmp $R0 "success" 0 +3
        ExecWait "$TEMP\vcredist_x86.exe"
        Goto +2
        MessageBox MB_OK "vcredist_x86.exe download failed: $R0"
    ${EndIf}

    notinstall2013:

FunctionEnd
