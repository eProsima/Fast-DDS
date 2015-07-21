macro(generate_msvc_libraries platform)
    string(COMPARE EQUAL "${platform}" "i86Win32VS2010" IS_I86WIN32VS2010)
    string(COMPARE EQUAL "${platform}" "x64Win64VS2010" IS_X64WIN64VS2010)
    string(COMPARE EQUAL "${platform}" "i86Win32VS2013" IS_I86WIN32VS2013)
    string(COMPARE EQUAL "${platform}" "x64Win64VS2013" IS_X64WIN64VS2013)

    set(CHECKGENERATOR "")
    if(IS_I86WIN32VS2010)
        set(GENERATOR "Visual Studio 10 2010")
    elseif(IS_X64WIN64VS2010)
        set(GENERATOR "Visual Studio 10 2010 Win64")
    elseif(IS_I86WIN32VS2013)
        set(GENERATOR "Visual Studio 12 2013")
    elseif(IS_X64WIN64VS2013)
        set(GENERATOR "Visual Studio 12 2013 Win64")
    else()
        message(FATAL_ERROR "Lexical error defining platform. Trying to use platform \"${platform}\"")
    endif()

    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/../${platform}")

    execute_process(COMMAND cmake -G "${GENERATOR}" -DEPROSIMA_BUILD=${EPROSIMA_BUILD} ../..
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../${platform}
        RESULT_VARIABLE EXECUTE_RESULT)

    if(NOT EXECUTE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed creating build for ${platform}")
    endif()

    execute_process(COMMAND cmake --build . --config Release
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../${platform}
        RESULT_VARIABLE EXECUTE_RESULT)

    if(NOT EXECUTE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed compiling build of ${platform} in Release")
    endif()

    execute_process(COMMAND ctest --config Release
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../${platform}
        RESULT_VARIABLE EXECUTE_RESULT)

    if(NOT EXECUTE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed testing build of ${platform} in Release")
    endif()

    execute_process(COMMAND cmake --build . --config Debug
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../${platform}
        RESULT_VARIABLE EXECUTE_RESULT)

    if(NOT EXECUTE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed compiling build of ${platform} in Debug")
    endif()

    execute_process(COMMAND ctest --config Debug
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../${platform}
        RESULT_VARIABLE EXECUTE_RESULT)

    if(NOT EXECUTE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed testing build of ${platform} in Debug")
    endif()

    if(NOT EXISTS ${PROJECT_BINARY_DIR}/../${platform}/lib/${PROJECT_NAME}-${PROJECT_VERSION}.dll)
        message(FATAL_ERROR "Not compiled Release version of ${platform} libraries")
    endif()
    if(NOT EXISTS ${PROJECT_BINARY_DIR}/../${platform}/lib/${PROJECT_NAME}d-${PROJECT_VERSION}.dll)
        message(FATAL_ERROR "Not compiled Debug version of ${platform} libraries")
    endif()
endmacro()
