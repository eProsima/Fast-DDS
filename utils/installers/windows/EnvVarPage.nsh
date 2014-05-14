!include nsDialogs.nsh
!include LogicLib.nsh

Var Dialog
Var FirstTime

Var Label

Var CheckboxEPROSIMARTPSHOME
Var CheckboxEPROSIMARTPSHOME_State

Var CheckboxScripts
Var CheckboxScripts_State

Var CheckboxX64
Var CheckboxX64_State

Var CheckboxI86
Var CheckboxI86_State

LangString PAGE_TITLE ${LANG_ENGLISH} "Environment variables setting"
LangString PAGE_SUBTITLE ${LANG_ENGLISH} "Choose which environment variables you want to set."

Function VariablesEntornoPage

    !insertmacro MUI_HEADER_TEXT $(PAGE_TITLE) $(PAGE_SUBTITLE)

    nsDialogs::Create 1018
    
    Pop $Dialog

    ${If} $Dialog == error
        Abort
    ${EndIf}
       
    ### Creación de los elementos gráficos    
    ${NSD_CreateLabel} 0 0 100% 20u "Check the environment variables you want to set and uncheck the environment variables you don't want to set. Click Next to continue."
    Pop $Label

    ${NSD_CreateCheckbox} 10 20u 100% 12u "Set the EPROSIMARTPSHOME environment variable."
    Pop $CheckboxEPROSIMARTPSHOME
    ${If} $CheckboxEPROSIMARTPSHOME_State == ${BST_CHECKED}
        ${NSD_Check} $CheckboxEPROSIMARTPSHOME
    ${EndIf}
        
    ${NSD_CreateCheckbox} 10 32u 100% 12u "&Add to the PATH environment variable the location of eProsima RTPS scripts"
    Pop $CheckboxScripts
    ${If} $CheckboxScripts_State == ${BST_CHECKED}
        ${NSD_Check} $CheckboxScripts
    ${EndIf}
    
    ${If} ${RunningX64}
        ${NSD_CreateCheckbox} 10 44u 100% 24u "&Add to the PATH environment variable the location of eProsima RTPS target$\r$\nlibraries for platform x64"
        Pop $CheckboxX64  
        ${If} ${SectionIsSelected} ${SEC_LIB_x64}
            ${If} $CheckboxX64_State == ${BST_CHECKED}
                ${NSD_Check} $CheckboxX64
            ${EndIf}
        ${Else}
            ${NSD_AddStyle} $CheckboxX64 ${WS_DISABLED}
        ${EndIf}

        ### Fijamos los callbacks para cuando se haga click en los CheckBoxes
    ${NSD_OnClick} $CheckboxX64 ClickX64  
    
        ${NSD_CreateCheckbox} 10 66u 100% 24u "&Add to the PATH environment variable the location of eProsima RTPS target$\r$\nlibraries for platform i86"
        Pop $CheckboxI86
    ${Else}
        ${NSD_CreateCheckbox} 10 44u 100% 24u "&Add to the PATH environment variable the location of eProsima RTPS target$\r$\nlibraries for platform i86"
        Pop $CheckboxI86
    ${EndIf}

    ${If} ${SectionIsSelected} ${SEC_LIB_i86}
        ${If} $CheckboxI86_State == ${BST_CHECKED}
            ${NSD_Check} $CheckboxI86
        ${EndIf}
    ${Else}
        ${NSD_AddStyle} $CheckboxI86 ${WS_DISABLED}
    ${EndIf}
    
    ### La primera vez que lanzamos el instalador, el checkbox de EPROSIMARTPSHOME
    ### y el de SCRIPTS deben estar marcados. 
    StrCmp $FirstTime "FirstTime" +5 0 ### Si son iguales las cadenas, GOTO +5, si no, GOTO 0
        ${NSD_Check} $CheckboxEPROSIMARTPSHOME
        ${NSD_Check} $CheckboxScripts
        ${NSD_GetState} $CheckboxEPROSIMARTPSHOME $CheckboxEPROSIMARTPSHOME_State
        ${NSD_GetState} $CheckboxScripts $CheckboxScripts_State
        StrCpy $FirstTime "FirstTime"
        
    ### Fijamos los callbacks para cuando se haga click en los CheckBoxes
    ${NSD_OnClick} $CheckboxEPROSIMARTPSHOME ClickEPROSIMARTPSHOME 
    ${NSD_OnClick} $CheckboxScripts ClickScripts
    ${NSD_OnClick} $CheckboxI86 ClickI86  

    nsDialogs::Show
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox EPROSIMARTPSHOME
### Guardamos el estado en la variable _state
Function ClickEPROSIMARTPSHOME
    ${NSD_GetState} $CheckboxEPROSIMARTPSHOME $CheckboxEPROSIMARTPSHOME_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox Scripts
### Guardamos el estado en la variable _state
Function ClickScripts
    ${NSD_GetState} $CheckboxScripts $CheckboxScripts_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox x64
### Sirve para deshabilitar el i86, pues no pueden aparecer a la vez
### También guardamos el estado en la variable _state
Function ClickX64
    Pop $CheckboxX64
    ${NSD_GetState} $CheckboxX64 $0
    ${If} $0 == 1
        ${NSD_SetState} $CheckboxI86 0
        ${NSD_GetState} $CheckboxI86 $CheckboxI86_State
    ${EndIf}
    ${NSD_GetState} $CheckboxX64 $CheckboxX64_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox i86
### Sirve para deshabilitar el x64, pues no pueden aparecer a la vez
### También guardamos el estado en la variable _state
Function ClickI86
    Pop $CheckboxI86
    ${NSD_GetState} $CheckboxI86 $0
    ${If} $0 == 1
        ${NSD_SetState} $CheckboxX64 0
        ${NSD_GetState} $CheckboxX64 $CheckboxX64_State
    ${EndIf}
    ${NSD_GetState} $CheckboxI86 $CheckboxI86_State
FunctionEnd
