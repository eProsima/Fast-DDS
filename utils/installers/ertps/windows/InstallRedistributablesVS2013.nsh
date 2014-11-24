!include x64.nsh

Var X64NeededVS2013
Var I86NeededVS2013

Function InstallRedistributablesVS2013

    StrCpy $X64NeededVS2013 "1"
    StrCpy $I86NeededVS2013 "1"

    # Check if it is necessary to install to x64
    ${If} ${RunningX64}
        ${If} ${SectionIsSelected} ${SEC_LIB_x64_VS2013}
            ReadRegStr $X64NeededVS2013 HKLM "SOFTWARE\Microsoft\VisualStudio\12.0\VC\VCRedist\x64" "Installed"
        ${EndIf}
    ${EndIf}

    # Check if it is necessary to install to i86
    ${If} ${SectionIsSelected} ${SEC_LIB_i86_VS2013}
        ReadRegStr $I86NeededVS2013 HKLM "SOFTWARE\Microsoft\VisualStudio\12.0\VC\VCRedist\x86" "Installed"
    ${EndIf}

    ${If} ${RunningX64}
        StrCmp $X64NeededVS2013 "1" 0 install
    ${EndIf}
    StrCmp $I86NeededVS2013 "1" notinstall install

    install:
    messageBox MB_YESNO|MB_ICONQUESTION "fastrtps needs Visual Studio 2013 Redistributable packages.$\nDo you want to install it?" IDNO notinstall

    ${If} $X64NeededVS2013 != "1"
        ExecWait "$TEMP\vcredist_x64_2013.exe"
    ${EndIf}

    ${If} $i86NeededVS2013 != "1"
        ExecWait "$TEMP\vcredist_x86_2013.exe"
    ${EndIf}

    notinstall:
FunctionEnd
