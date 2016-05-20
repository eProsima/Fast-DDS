macro(compile_example example example_directory)
    if(NOT ((EPROSIMA_INSTALLER OR EPROSIMA_INSTALLER_MINION) AND (MSVC OR MSVC_IDE)))

        # Check if example use boost
        set(USE_BOOST_ "")
        foreach(arg ${ARGN})
            if("${arg}" STREQUAL "USE_BOOST")
                set(USE_BOOST_ "-DBOOST_ROOT:PATH=${BOOST_ROOT}" "-DBOOST_LIBRARYDIR:PATH=${BOOST_LIBRARYDIR}")
            endif()
        endforeach()

        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config/fastrtpsConfig.cmake
            "include(\"${PROJECT_BINARY_DIR}/cmake/config/fastrtpsTargets.cmake\")\n"
            )

        # Separate CMAKE_PREFIX_PATH
        string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ "${CMAKE_PREFIX_PATH}")
        set(${example}_CMAKE_ARGS
            "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}"
            "-DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/config|${CMAKE_PREFIX_PATH_}"
            ${USE_BOOST_}
            "-DBIN_INSTALL_DIR:PATH=${BIN_INSTALL_DIR}")
        list(APPEND ${example}_CMAKE_ARGS LIST_SEPARATOR "|")

        include(ExternalProject)

        ExternalProject_Add(${example}
            DEPENDS ${PROJECT_NAME}
            CMAKE_GENERATOR "${CMAKE_GENERATOR}"
            CMAKE_ARGS
            ${${example}_CMAKE_ARGS}
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${example_directory}"
            BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${example_directory}"
            INSTALL_COMMAND ""
            )
    endif()
endmacro()
