!include nsDialogs.nsh
!include LogicLib.nsh

Var Dialog
Var FirstTime

Var Label

Var CheckboxfastrtpsHOME
Var CheckboxfastrtpsHOME_State

Var CheckboxScripts
Var CheckboxScripts_State

Var CheckboxRadioButtons
Var CheckboxRadioButtons_State

Var GroupBoxRadioButton

Var RadioButtonX64_VS2010
Var RadioButtonX64_VS2010_State

Var RadioButtonI86_VS2010
Var RadioButtonI86_VS2010_State

Var RadioButtonX64_VS2013
Var RadioButtonX64_VS2013_State

Var RadioButtonI86_VS2013
Var RadioButtonI86_VS2013_State

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
        
    ${NSD_CreateCheckbox} 10 32u 100% 12u "&Add to the PATH the location of eProsima FASTRTPSGEN scripts."
    Pop $CheckboxScripts
    ${If} $CheckboxScripts_State == ${BST_CHECKED}
        ${NSD_Check} $CheckboxScripts
    ${EndIf}
	
	${NSD_CreateCheckbox} 10 50u 100% 12u "&Add to the PATH the location of eProsima Fast RTPS libraries."
    Pop $CheckboxRadioButtons
    ${If} $CheckboxRadioButtons_State == ${BST_CHECKED}
        ${NSD_Check} $CheckboxRadioButtons
    ${EndIf}
	
	### La primera vez que lanzamos el instalador, el checkbox de fastrtpsHOME
    ### y el de SCRIPTS deben estar marcados. 
    StrCmp $FirstTime "FirstTime" +5 0 ### Si son iguales las cadenas, GOTO +5, si no, GOTO 0
        ${NSD_Check} $CheckboxfastrtpsHOME
        ${NSD_Check} $CheckboxScripts
		${NSD_Check} $CheckboxRadioButtons
        ${NSD_GetState} $CheckboxfastrtpsHOME $CheckboxfastrtpsHOME_State
        ${NSD_GetState} $CheckboxScripts $CheckboxScripts_State
		${NSD_GetState} $CheckboxRadioButtons $CheckboxRadioButtons_State
        StrCpy $FirstTime "FirstTime"
	
	
	!define lib_radio_x 30
	!define lib_radio_height 15u
	!define lib_radio_y_0 75u
	!define lib_radio_y_1 90u
	!define lib_radio_y_2 105u
	!define lib_radio_y_3 120u
	
	; GroupBox 1     
	${NSD_CreateGroupBox} 20 65u 90% 75u "Select the configuration:" 
	Pop $GroupBoxRadioButton   
	#Pop $GB_Ar1 
		
    
    ${If} ${RunningX64}
		 ${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_0} 100% ${lib_radio_height} "&x64 VS2013 platforms."
        Pop $RadioButtonX64_VS2013  
        ### Fijamos los callbacks para cuando se haga click en los RadioButtones
		${NSD_OnClick} $RadioButtonX64_VS2013 ClickX64_VS2013  
	
        ${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_1} 100% ${lib_radio_height} "&x64 VS2010 platforms."
        Pop $RadioButtonX64_VS2010  
        ### Fijamos los callbacks para cuando se haga click en los RadioButtones
		${NSD_OnClick} $RadioButtonX64_VS2010 ClickX64_VS2010  

		${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_2} 100% ${lib_radio_height} "&i86 VS2013 platforms."
        Pop $RadioButtonI86_VS2013
        ${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_3} 100% ${lib_radio_height} "&i86 VS2010 platforms."
        Pop $RadioButtonI86_VS2010
		
		Call EnableRadioX64
		Call EnableRadioI86	
    ${Else}
		${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_0} 100% ${lib_radio_height} "&i86 VS2013 platforms."
		Pop $RadioButtonI86_VS2013
        ${NSD_CreateRadioButton} ${lib_radio_x} ${lib_radio_y_1} 100% ${lib_radio_height} "&i86 VS2010 platforms."
        Pop $RadioButtonI86_VS2010
		
		Call EnableRadioI86	
    ${EndIf}

    
        
    ### Fijamos los callbacks para cuando se haga click en los CheckBoxes
    ${NSD_OnClick} $CheckboxfastrtpsHOME ClickfastrtpsHOME 
    ${NSD_OnClick} $CheckboxScripts ClickScripts
	${NSD_OnClick} $CheckboxRadioButtons ClickCheckboxRadioButtons
    ${NSD_OnClick} $RadioButtonI86_VS2010 ClickI86_VS2010  
	${NSD_OnClick} $RadioButtonI86_VS2013 ClickI86_VS2013 
	
	Call SelectDefaultBestConfiguration
	
	
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


Function ClickCheckboxRadioButtons
    ${NSD_GetState} $CheckboxRadioButtons $CheckboxRadioButtons_State
	${If} $CheckboxRadioButtons_State == 0
		 EnableWindow $RadioButtonX64_VS2010 0
		 EnableWindow $RadioButtonI86_VS2010 0
		 EnableWindow $RadioButtonX64_VS2013 0
		 EnableWindow $RadioButtonI86_VS2013 0
		 
		 ${NSD_SetState} $RadioButtonX64_VS2010 0
		 ${NSD_SetState} $RadioButtonI86_VS2010 0
		 ${NSD_SetState} $RadioButtonX64_VS2013 0
		 ${NSD_SetState} $RadioButtonI86_VS2013 0

		 ${NSD_GetState} $RadioButtonI86_VS2010 $RadioButtonI86_VS2010_State
		 ${NSD_GetState} $RadioButtonX64_VS2010 $RadioButtonX64_VS2010_State
		 ${NSD_GetState} $RadioButtonX64_VS2013 $RadioButtonX64_VS2013_State
		 ${NSD_GetState} $RadioButtonI86_VS2013 $RadioButtonI86_VS2013_State
	${ElseIf} $CheckboxRadioButtons_State == 1
		Call EnableRadioX64
		Call EnableRadioI86
		Call SelectDefaultBestConfiguration
	${EndIf}
FunctionEnd


### Callback invocado cuando se pulsa el CheckBox x64
### Sirve para deshabilitar el i86, pues no pueden aparecer a la vez
### Tambi�n guardamos el estado en la variable _state
Function ClickX64_VS2010
    Pop $RadioButtonX64_VS2010
    ${NSD_GetState} $RadioButtonX64_VS2010 $RadioButtonX64_VS2010_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox i86
### Sirve para deshabilitar el x64, pues no pueden aparecer a la vez
### Tambi�n guardamos el estado en la variable _state
Function ClickI86_VS2010
    Pop $RadioButtonI86_VS2010
    ${NSD_GetState} $RadioButtonI86_VS2010 $RadioButtonI86_VS2010_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox x64 VS2013
### Sirve para deshabilitar el i86 2013, pues no pueden aparecer a la vez
### Tambi�n guardamos el estado en la variable _state
Function ClickX64_VS2013
    Pop $RadioButtonX64_VS2013
    ${NSD_GetState} $RadioButtonX64_VS2013 $RadioButtonX64_VS2013_State
FunctionEnd

### Callback invocado cuando se pulsa el CheckBox i86
### Sirve para deshabilitar el x64, pues no pueden aparecer a la vez
### Tambi�n guardamos el estado en la variable _state
Function ClickI86_VS2013
    Pop $RadioButtonI86_VS2013
    ${NSD_GetState} $RadioButtonI86_VS2013 $RadioButtonI86_VS2013_State
FunctionEnd

Function SelectDefaultBestConfiguration
	${NSD_GetState} $CheckboxRadioButtons $CheckboxRadioButtons_State
	${If} $CheckboxRadioButtons_State == 1
		${If} ${SectionIsSelected} ${SEC_LIB_x64VS2013}
			${NSD_SetState} $RadioButtonX64_VS2013 1
			${NSD_GetState} $RadioButtonX64_VS2013 $RadioButtonX64_VS2013_State
		${ElseIf} ${SectionIsSelected} ${SEC_LIB_x64VS2010}	
			${NSD_SetState} $RadioButtonX64_VS2010 1
			${NSD_GetState} $RadioButtonX64_VS2010 $RadioButtonX64_VS2010_State
		${ElseIf} ${SectionIsSelected} ${SEC_LIB_i86VS2013}	
			${NSD_SetState} $RadioButtonI86_VS2013 1
			${NSD_GetState} $RadioButtonI86_VS2013 $RadioButtonI86_VS2013_State
		${ElseIf} ${SectionIsSelected} ${SEC_LIB_i86VS2010}	
			${NSD_SetState} $RadioButtonI86_VS2010 1
			${NSD_GetState} $RadioButtonI86_VS2010 $RadioButtonI86_VS2010_State		
		${EndIf}
	${EndIf}
FunctionEnd

Function EnableRadioX64
${NSD_GetState} $CheckboxRadioButtons $CheckboxRadioButtons_State
	${If} $CheckboxRadioButtons_State == 1
 ${If} ${SectionIsSelected} ${SEC_LIB_x64VS2013}
		EnableWindow $RadioButtonX64_VS2013 1	
 ${Else}
		EnableWindow $RadioButtonX64_VS2013 0
 ${EndIf}
 ${If} ${SectionIsSelected} ${SEC_LIB_x64VS2010}
		EnableWindow $RadioButtonX64_VS2010 1	
 ${Else}
		EnableWindow $RadioButtonX64_VS2010 0
 ${EndIf}
 ${Else}
 EnableWindow $RadioButtonX64_VS2013 0
 EnableWindow $RadioButtonX64_VS2010 0
 ${EndIf}
FunctionEnd

Function EnableRadioI86
${NSD_GetState} $CheckboxRadioButtons $CheckboxRadioButtons_State
	${If} $CheckboxRadioButtons_State == 1
 ${If} ${SectionIsSelected} ${SEC_LIB_i86VS2013}
		EnableWindow $RadioButtonI86_VS2013 1	
 ${Else}
		EnableWindow $RadioButtonI86_VS2013 0
 ${EndIf}
 ${If} ${SectionIsSelected} ${SEC_LIB_i86VS2010}
		EnableWindow $RadioButtonI86_VS2010 1	
 ${Else}
		EnableWindow $RadioButtonI86_VS2010 0
 ${EndIf}
 ${Else}
 EnableWindow $RadioButtonI86_VS2013 0
 EnableWindow $RadioButtonI86_VS2010 0
  ${EndIf}
FunctionEnd