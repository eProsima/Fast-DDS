macro(gradle_build directory)
    find_package(Java 1.6 COMPONENTS Runtime REQUIRED)
    if(WIN32)
        find_program(GRADLE_EXE gradle.bat)
    else()
        find_program(GRADLE_EXE gradle)
    endif()

    if(GRADLE_EXE)
        message(STATUS "Found Gradle: ${GRADLE_EXE}")
    else()
        message(FATAL_ERROR "gradle is needed to build the java application. Please install it correctly")
    endif()

    get_filename_component(Java_JAVA_EXECUTABLE_DIR ${Java_JAVA_EXECUTABLE} DIRECTORY)
    file(TO_NATIVE_PATH "${Java_JAVA_EXECUTABLE_DIR}" Java_JAVA_EXECUTABLE_DIR_NATIVE)

    if(WIN32)
        set(delimiter_ ";")
    else()
        set(delimiter_ ":")
    endif()

    add_custom_target(java ALL
        COMMAND ${CMAKE_COMMAND} -E env
        --unset=JAVA_HOME
        "PATH=${Java_JAVA_EXECUTABLE_DIR_NATIVE}${delimiter_}$<JOIN:$ENV{PATH},${delimiter_}>"
        "${GRADLE_EXE}" -Pcustomversion=${PROJECT_VERSION} build
        WORKING_DIRECTORY ${directory}
        COMMENT "Generating Java application" VERBATIM)

    set(THIRDPARTY_FOUND false)
    foreach(arg ${ARGN})
        if("${arg}" STREQUAL "THIRDPARTY_DEPENDENCY")
            set(THIRDPARTY_FOUND true)
        else()
            if(THIRDPARTY_FOUND)
                add_custom_target(git_submodule_update_${arg}
                    COMMAND git submodule update --recursive --init thirdparty/${arg}
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    COMMENT "Updating Git module ${arg}" VERBATIM)
                add_dependencies(java git_submodule_update_${arg})
                set(THIRDPARTY_FOUND false)
            else()
                message(FATAL_ERROR "Bad use of gradle_build command")
            endif()
        endif()
    endforeach()
endmacro()
