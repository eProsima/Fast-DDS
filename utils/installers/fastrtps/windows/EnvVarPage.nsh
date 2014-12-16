!include nsDialogs.nsh
!include LogicLib.nsh

Var Dialog
Var FirstTime

Var Label

Var CheckboxfastrtpsHOME
Var CheckboxfastrtpsHOME_State

Var CheckboxScripts
Var CheckboxScripts_State

Var CheckboxX64_VS2010
Var CheckboxX64_VS2010_State

Var CheckboxI86_VS2010
Var CheckboxI86_VS2010_State

Var CheckboxX64_VS2013
Var CheckboxX64_VS2013_State

Var CheckboxI86_VS2013
Var CheckboxI86_VS2013_State

LangString PAGE_TITLE ${LANG_ENGLISH} "Environment variables setting"
LangString PAGE_SUBTITLE ${LANG_ENGLISH} "Choose which environment variables you want to set."

Function VariablesEntornoPage

    !insertmacro MUI_HEADER_TEXT $(PAGE_TITLE) $(PAGE_SUBTITLE)

    nsDialogs::Create 1018
    
    Pop $Dialog

    ${If} $Dialog == error
        Abort
    ${EndIf}
       
    ### Creaci�n de los elementos gr�ficos    
    ${NSD_CreateLabel} 0 0 100% 20u "Check the environment variables you want to set and uncheck the environment variables you don't want to set. Click Next to continue."
    Pop $Label

    ${NSD_CreateCheckbox} 10 20u 100% 12u "Set the FASTRTPSHOME environment variable."
    Pop $CheckboxfastrtpsHOME
    ${If} $CheckboxfastrtpsHOME_State == ${BST_CHECKED}
        ${NSD_Check} $CheckboxfastrtpsHOME
    ${EndIf}
        
    ${NSD_CreateCheckbox} 10 32u 100% 12u "&Add to the PATH  the location of eProsima FASTRTPSGEN scripts."
    Pop $CheckboxScripts
    ${If} $CheckboxScripts_State == ${BST_CHECKED}
        ${NSD_Check} $CheckboxScripts
    ${EndIf}
	
	
	!define lib_radio_x 20
	!define lib_radio_height 15u
	!define lib_radio_y_0 60u
	!define lib_radio_y_1 75u
	!define lib_radio_y_2 90u
	!define lib_radio_y_3 105u
	
	; GroupBox 1     
	${NSD_CreateGroupBox} 10 50u 95% 75u "Add to the PATH the location of eProsima Fast RTPS libraries for:"     
	#Pop $GB_Ar1 
		
    
    ${If} ${RunningX64}
        ${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_0} 100% ${lib_radio_height} "&x64 VS2010 platforms."
        Pop $CheckboxX64_VS2010  
        ${If} ${SectionIsSelected} ${SEC_LIB_x64VS2010}
            ${If} $CheckboxX64_VS2010_State == ${BST_CHECKED}
                ${NSD_Check} $CheckboxX64_VS2010
            ${EndIf}
        ${Else}
            ${NSD_AddStyle} $CheckboxX64_VS2010 ${WS_DISABLED}
        ${EndIf}
        ### Fijamos los callbacks para cuando se haga click en los CheckBoxes
		${NSD_OnClick} $CheckboxX64_VS2010 ClickX64_VS2010  
    
		 ${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_1} 100% ${lib_radio_height} "&x64 VS2013 platforms."
        Pop $CheckboxX64_VS2013  
        ${If} ${SectionIsSelected} ${SEC_LIB_x64VS2013}
            ${If} $CheckboxX64_VS2013_State == ${BST_CHECKED}
                ${NSD_Check} $CheckboxX64_VS2013
            ${EndIf}
        ${Else}
            ${NSD_AddStyle} $CheckboxX64_VS2013 ${WS_DISABLED}
        ${EndIf}
        ### Fijamos los callbacks para cuando se haga click en los CheckBoxes
		${NSD_OnClick} $CheckboxX64_VS2013 ClickX64_VS2013  
	
        ${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_2} 100% ${lib_radio_height} "&i86 VS2010 platforms."
        Pop $CheckboxI86_VS2010
		${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_3} 100% ${lib_radio_height} "&i86 VS2013 platforms."
        Pop $CheckboxI86_VS2013
    ${Else}
        ${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_0} 100% ${lib_radio_height} "&i86 VS2010 platforms."
        Pop $CheckboxI86_VS2010
		${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_1} 100% ${lib_radio_height} "&i86 VS2013 platforms."
        Pop $CheckboxI86_VS2013
    ${EndIf}

    ${If} ${SectionIsSelected} ${SEC_LIB_i86VS2010}
        ${If} $CheckboxI86_VS2010_State == ${BST_CHECKED}
            ${NSD_Check} $CheckboxI86_VS2010
        ${EndIf}
    ${Else}
        ${NSD_AddStyle} $CheckboxI86_VS2010 ${WS_DISABLED}
    ${EndIf}
	${If} ${SectionIsSelected} ${SEC_LIB_i86VS2013}
        ${If} $CheckboxI86_VS2013_State == ${BST_CHECKED}
            ${NSD_Check} $CheckboxI86_VS2013
        ${EndIf}
    ${Else}
        ${NSD_AddStyle} $CheckboxI86_VS2013 ${WS_DISABLED}
    ${EndIf}	
    
    ### La primera vez que lanzamos el instalador, el checkbox de fastrtpsHOME
    ### y el de SCRIPTS deben estar marcados. 
    StrCmp $FirstTime "FirstTime" +5 0 ### Si son iguales las cadenas, GOTO +5, si no, GOTO 0
        ${NSD_Check} $CheckboxfastrtpsHOME
        ${NSD_Check} $CheckboxScripts
        ${NSD_GetState} $CheckboxfastrtpsHOME $CheckboxfastrtpsHOME_State
        ${NSD_GetState} $CheckboxScripts $CheckboxScripts_State
        StrCpy $FirstTime "FirstTime"
        
    ### Fijamos los callbacks para cuando se haga click en los CheckBoxes
    ${NSD_OnClick} $CheckboxfastrtpsHOME ClickfastrtpsHOME 
    ${NSD_OnClick} $CheckboxScripts ClickScripts
    ${NSD_OnClick} $CheckboxI86_VS2010 ClickI86_VS2010  
	${NSD_OnClick} $CheckboxI86_VS2013 ClickI86_VS2013 

    nsDialogs::Show
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox fastrtpsHOME
### Guardamos el estado en la variable _state
Function ClickfastrtpsHOME
    ${NSD_GetState} $CheckboxfastrtpsHOME $CheckboxfastrtpsHOME_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox Scripts
### Guardamos el estado en la variable _state
Function ClickScripts
    ${NSD_GetState} $CheckboxScripts $CheckboxScripts_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox x64
### Sirve para deshabilitar el i86, pues no pueden aparecer a la vez
### Tambi�n guardamos el estado en la variable _state
Function ClickX64_VS2010
    Pop $CheckboxX64_VS2010
    ${NSD_GetState} $CheckboxX64_VS2010 $CheckboxX64_VS2010_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox i86
### Sirve para deshabilitar el x64, pues no pueden aparecer a la vez
### Tambi�n guardamos el estado en la variable _state
Function ClickI86_VS2010
    Pop $CheckboxI86_VS2010
    ${NSD_GetState} $CheckboxI86_VS2010 $CheckboxI86_VS2010_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox x64 VS2013
### Sirve para deshabilitar el i86 2013, pues no pueden aparecer a la vez
### Tambi�n guardamos el estado en la variable _state
Function ClickX64_VS2013
    Pop $CheckboxX64_VS2013
    ${NSD_GetState} $CheckboxX64_VS2013 $CheckboxX64_VS2013_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox i86
### Sirve para deshabilitar el x64, pues no pueden aparecer a la vez
### Tambi�n guardamos el estado en la variable _state
Function ClickI86_VS2013
    Pop $CheckboxI86_VS2013
    ${NSD_GetState} $CheckboxI86_VS2013 $CheckboxI86_VS2013_State
FunctionEnd
