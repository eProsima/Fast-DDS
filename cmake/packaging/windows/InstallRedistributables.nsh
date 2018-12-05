# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

!include x64.nsh

Var X64VS2017Needed
Var I86VS2017Needed

Function InstallRedistributables

    StrCpy $X64VS2017Needed "1"
    StrCpy $I86VS2017Needed "1"

    ${If} ${RunningX64}
        SetRegView 64
    ${EndIf}

    # Check if it is necessary to install to x64VS2017
    ${If} ${RunningX64}
        ${If} ${SectionIsSelected} ${libraries_x64Win64VS2017}
        ${OrIf} ${SectionIsSelected} ${libraries_x64Win64VS2015}
            ClearErrors
            ReadRegStr $0 HKLM "SOFTWARE\Classes\Installer\Dependencies\Microsoft.VS.VC_RuntimeMinimumVSU_amd64,v14" "Version"
            IfErrors 0 VC2017x64RedistInstalled
            StrCpy $X64VS2017Needed "0"
        ${EndIf}
    ${EndIf}

    VC2017x64RedistInstalled:

    # Check if it is necessary to install to i86VS2017
    ${If} ${SectionIsSelected} ${libraries_i86Win32VS2017}
    ${OrIf} ${SectionIsSelected} ${libraries_i86Win32VS2015}
        ClearErrors
        ReadRegStr $0 HKLM "SOFTWARE\Classes\Installer\Dependencies\Microsoft.VS.VC_RuntimeMinimumVSU_x86,v14" "Version"
        IfErrors 0 VC2017i86RedistInstalled
        StrCpy $I86VS2017Needed "0"
    ${EndIf}

    VC2017i86RedistInstalled:

    StrCmp $X64VS2017Needed "1" notinstall2017x64 install2017x64

    install2017x64:
    messageBox MB_YESNO|MB_ICONQUESTION "$(^Name) needs Visual Studio 2017 x64 Redistributable packages.$\nDo you want to download and install them?" IDNO notinstall2017x64

    NSISdl::download https://aka.ms/vs/15/release/VC_redist.x64.exe $TEMP\VC_redist.x64.exe
    Pop $R0 ; Get the return value
    StrCmp $R0 "success" 0 +3
    ExecWait "$TEMP\VC_redist.x64.exe"
    Goto +2
    MessageBox MB_OK "VC_redist.x64.exe download failed: $R0"

    notinstall2017x64:

    StrCmp $I86VS2017Needed "1" notinstall2017i86 install2017i86

    install2017i86:
    messageBox MB_YESNO|MB_ICONQUESTION "$(^Name) needs Visual Studio 2017 Win32 Redistributable packages.$\nDo you want to download and install them?" IDNO notinstall2017i86

    NSISdl::download https://aka.ms/vs/15/release/VC_redist.x86.exe $TEMP\VC_redist.x86.exe
    Pop $R0 ; Get the return value
    StrCmp $R0 "success" 0 +3
    ExecWait "$TEMP\VC_redist.x86.exe"
    Goto +2
    MessageBox MB_OK "VC_redist.x86.exe download failed: $R0"

    notinstall2017i86:

FunctionEnd
