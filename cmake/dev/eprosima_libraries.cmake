macro(find_eprosima_package package)
    if(NOT (EPROSIMA_INSTALLER AND (MSVC OR MSVC_IDE)))
        if(THIRDPARTY)
            set(USE_BOOST_ "")
            foreach(arg ${ARGN})
                if("${arg}" STREQUAL "USE_BOOST")
                    set(USE_BOOST_ "-DEPROSIMA_BOOST=${EPROSIMA_BOOST}")
                endif()
            endforeach()

            set(${package}ExternalDir ${PROJECT_BINARY_DIR}/external/${package})

            if(MINION)
                set(CMAKE_INSTALL_PREFIX_ "${CMAKE_INSTALL_PREFIX}")
            else()
                set(CMAKE_INSTALL_PREFIX_ "${PROJECT_BINARY_DIR}/external/install")
            endif()

            # Separate CMAKE_PREFIX_PATH
            string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ "${CMAKE_PREFIX_PATH}")
            set(${package}_CMAKE_ARGS
                "\${SOURCE_DIR_}"
                "\${GENERATOR_}"
                ${BUILD_OPTION}
                "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
                ${USE_BOOST_}
                "-DMINION=ON"
                "-DEPROSIMA_INSTALLER_MINION=${EPROSIMA_INSTALLER_MINION}"
                "-DBIN_INSTALL_DIR:PATH=${BIN_INSTALL_DIR}"
                "-DINCLUDE_INSTALL_DIR:PATH=${INCLUDE_INSTALL_DIR}"
                "-DLIB_INSTALL_DIR:PATH=${LIB_INSTALL_DIR}"
                "-DLICENSE_INSTALL_DIR:PATH=licenses"
                "\${CMAKE_INSTALL_PREFIX_}"
                "\${CMAKE_PREFIX_PATH_}"
                )
            list(APPEND ${package}_CMAKE_ARGS LIST_SEPARATOR "|")

            file(MAKE_DIRECTORY ${${package}ExternalDir})
            file(WRITE ${${package}ExternalDir}/CMakeLists.txt
                "cmake_minimum_required(VERSION 2.8.12)\n"
                "include(ExternalProject)\n"
                "set(SOURCE_DIR_ \"${PROJECT_SOURCE_DIR}/thirdparty/${package}\")\n"
                "set(GENERATOR_ -G \"${CMAKE_GENERATOR}\")\n"
                "set(CMAKE_INSTALL_PREFIX_ \"-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX_}\")\n"
                "set(CMAKE_PREFIX_PATH_ -DCMAKE_PREFIX_PATH=\"${CMAKE_PREFIX_PATH_}\")\n"
                "ExternalProject_Add(${package}\n"
                "CONFIGURE_COMMAND \"${CMAKE_COMMAND}\"\n"
                "${${package}_CMAKE_ARGS}\n"
                "DOWNLOAD_COMMAND \"\"\n"
                "UPDATE_COMMAND cd \"${PROJECT_SOURCE_DIR}\" && git submodule update --recursive --init \"thirdparty/${package}\"\n"
                "SOURCE_DIR \${SOURCE_DIR_}\n"
                "BINARY_DIR \"${${package}ExternalDir}/build\"\n"
                ")\n")

            execute_process(COMMAND ${CMAKE_COMMAND}
                -G ${CMAKE_GENERATOR}
                ${BUILD_OPTION}
                WORKING_DIRECTORY ${${package}ExternalDir}
                RESULT_VARIABLE EXECUTE_RESULT
                )

            if(NOT EXECUTE_RESULT EQUAL 0)
                message(FATAL_ERROR "Cannot configure Git submodule ${package}")
            endif()

            if(MSVC OR MSVC_IDE)
                if("${CMAKE_BUILD_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
                    set(BUILD_TYPE_GENERATION "Release")
                else()
                    set(BUILD_TYPE_GENERATION ${CMAKE_BUILD_TYPE})
                endif()

                execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
                    WORKING_DIRECTORY ${${package}ExternalDir}
                    RESULT_VARIABLE EXECUTE_RESULT
                    )

                if(NOT EXECUTE_RESULT EQUAL 0)
                    message(FATAL_ERROR "Cannot build Git submodule ${package} in Debug mode")
                endif()

                execute_process(COMMAND ${CMAKE_COMMAND} --build . --config ${BUILD_TYPE_GENERATION}
                    WORKING_DIRECTORY ${${package}ExternalDir}
                    RESULT_VARIABLE EXECUTE_RESULT
                    )

                if(NOT EXECUTE_RESULT EQUAL 0)
                    message(FATAL_ERROR "Cannot build Git submodule ${package} in ${BUILD_TYPE_GENERATION} mode")
                endif()
            else()
                execute_process(COMMAND ${CMAKE_COMMAND} --build .
                    WORKING_DIRECTORY ${${package}ExternalDir}
                    RESULT_VARIABLE EXECUTE_RESULT
                    )

                if(NOT EXECUTE_RESULT EQUAL 0)
                    message(FATAL_ERROR "Cannot build Git submodule ${package}")
                endif()
            endif()

            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_INSTALL_PREFIX_})
        endif()

        find_package(${package} QUIET)

        if(${package}_FOUND)
            message(STATUS "${package} library found...")
        else()
            message(STATUS "${package} library not found...")
        endif()

    endif()
endmacro()

macro(install_eprosima_libraries)
    if((MSVC OR MSVC_IDE) AND EPROSIMA_BUILD AND NOT MINION)
        if(EPROSIMA_INSTALLER)
            # Install includes. Take from x64Win64VS2013
            install(DIRECTORY ${PROJECT_BINARY_DIR}/eprosima_installer/x64Win64VS2015/install/${INCLUDE_INSTALL_DIR}/
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                OPTIONAL
                )

            # Install licenses. Take from x64Win64VS2013
            install(DIRECTORY ${PROJECT_BINARY_DIR}/eprosima_installer/x64Win64VS2015/install/licenses/
                DESTINATION ${LICENSE_INSTALL_DIR}
                COMPONENT licenses
                OPTIONAL
                )
        else()
            if("${CMAKE_BUILD_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
                set(BUILD_TYPE_INSTALLATION "Release")
            else()
                set(BUILD_TYPE_INSTALLATION ${CMAKE_BUILD_TYPE})
            endif()

            # Install includes
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${INCLUDE_INSTALL_DIR}/
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                OPTIONAL
                )

            # Install libraries
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${BIN_INSTALL_DIR}/
                DESTINATION ${BIN_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS Debug
                OPTIONAL
                FILES_MATCHING
                PATTERN "*d.*"
                PATTERN "*d-*.*"
                )

            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${BIN_INSTALL_DIR}/
                DESTINATION ${BIN_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS ${BUILD_TYPE_INSTALLATION}
                OPTIONAL
                FILES_MATCHING
                PATTERN "*"
                PATTERN "*d.*" EXCLUDE
                PATTERN "*d-*.*" EXCLUDE
                )

            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${LIB_INSTALL_DIR}/
                DESTINATION ${LIB_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS Debug
                OPTIONAL
                FILES_MATCHING
                PATTERN "*d.*"
                PATTERN "*d-*.*"
                )

            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${LIB_INSTALL_DIR}/
                DESTINATION ${LIB_INSTALL_DIR}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS ${BUILD_TYPE_INSTALLATION}
                OPTIONAL
                FILES_MATCHING
                PATTERN "*"
                PATTERN "*d.*" EXCLUDE
                PATTERN "*d-*.*" EXCLUDE
                )

            # Install licenses
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/licenses/
                DESTINATION ${LICENSE_INSTALL_DIR}
                COMPONENT licenses
                OPTIONAL
                )
        endif()
    elseif(UNIX AND EPROSIMA_BUILD AND NOT MINION AND NOT EPROSIMA_INSTALLER)
            # Install includes
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${INCLUDE_INSTALL_DIR}/
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                OPTIONAL
                )

            # Install libraries
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/${LIB_INSTALL_DIR}/
                DESTINATION ${LIB_INSTALL_DIR}
                USE_SOURCE_PERMISSIONS
                COMPONENT libraries
                )

            # Install licenses
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/licenses/
                DESTINATION ${LICENSE_INSTALL_DIR}
                COMPONENT licenses
                OPTIONAL
                )
    endif()
endmacro()
