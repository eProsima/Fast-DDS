!include x64.nsh

Var X64Needed
Var I86Needed

Function InstallRedistributables

    StrCpy $X64Needed "1"
    StrCpy $I86Needed "1"

    # Check if it is necessary to install to x64
    ${If} ${RunningX64}
        ${If} ${SectionIsSelected} ${SEC_LIB_x64}
            ReadRegStr $X64Needed HKLM "SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x64" "Installed"
        ${EndIf}
    ${EndIf}

    # Check if it is necessary to install to i86
    ${If} ${SectionIsSelected} ${SEC_LIB_i86}
        ReadRegStr $I86Needed HKLM "SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86" "Installed"
    ${EndIf}

    ${If} ${RunningX64}
        StrCmp $X64Needed "1" 0 install
    ${EndIf}
    StrCmp $I86Needed "1" notinstall install

    install:
    messageBox MB_YESNO|MB_ICONQUESTION "DynamicFastBuffers needs Visual Studio 2010 Redistributable packages.$\nDo you want to install it?" IDNO notinstall

    ${If} $X64Needed != "1"
        ExecWait "$TEMP\vcredist_x64.exe"
    ${EndIf}

    ${If} $i86Needed != "1"
        ExecWait "$TEMP\vcredist_x86.exe"
    ${EndIf}

    notinstall:
FunctionEnd
