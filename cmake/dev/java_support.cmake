macro(gradle_build directory)
    find_program(GRADLE_EXE gradle)
    if(GRADLE_EXE)
        message(STATUS "Found Gradle: ${GRADLE_EXE}")
    else()
        message(FATAL_ERROR "gradle is needed to build the java application. Please install it correctly")
    endif()

    add_custom_target(java ALL
        COMMAND "${GRADLE_EXE}" -Pcustomversion=${PROJECT_VERSION} build
        WORKING_DIRECTORY ${directory}
        COMMENT "Generating Java application" VERBATIM)

    set(THIRDPARTY_FOUND false)
    foreach(arg ${ARGN})
        if("${arg}" STREQUAL "THIRDPARTY_DEPENDENCY")
            set(THIRDPARTY_FOUND true)
        else()
            if(THIRDPARTY_FOUND)
                add_custom_command(TARGET java PRE_BUILD
                    COMMAND git submodule update --recursive --init thirdparty/${arg}
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    COMMENT "Updating Git module ${arg}" VERBATIM)
                set(THIRDPARTY_FOUND false)
            else()
                message(FATAL_ERROR "Bad use of gradle_build command")
            endif()
        endif()
    endforeach()
endmacro()
