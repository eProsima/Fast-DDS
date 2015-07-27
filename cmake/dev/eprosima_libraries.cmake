macro(find_eprosima_package package)
    if(EPROSIMA_BUILD)
        # Verify thirdparty
        message(STATUS "Checking ${package} thirdparty...")
        execute_process(COMMAND git submodule update --recursive --init thirdparty/${package}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            RESULT_VARIABLE EXECUTE_RESULT
            )

        if(NOT EXECUTE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed updating Git submodule ${package}")
        endif()

        include(ExternalProject)

        ExternalProject_Add(${package}
            CONFIGURE_COMMAND ${CMAKE_COMMAND}
            ${PROJECT_SOURCE_DIR}/thirdparty/${package}
            -G ${CMAKE_GENERATOR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/external-install
            SOURCE_DIR ${PROJECT_SOURCE_DIR}/thirdparty/${package}
            BINARY_DIR ${PROJECT_BINARY_DIR}/external-build/${package}
            )

        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PROJECT_BINARY_DIR}/external-install)
    endif()

    find_package(${package})
endmacro()

macro(install_eprosima_fastcdr)
    if(MSVC OR MSVC_IDE)
        # Install includes
        if(EPROSIMA_BUILD)
            install(DIRECTORY ${PROJECT_BINARY_DIR}/external-install/include/fastcdr 
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                )
        else()
            install(DIRECTORY ${fastcdr_INCLUDE_DIR}/fastcdr
                DESTINATION ${INCLUDE_INSTALL_DIR}
                COMPONENT headers
                )
        endif()

        if(EPROSIMA_INSTALLER)
            install(DIRECTORY ${PROJECT_BINARY_DIR}/../i86Win32VS2010/external-install/lib/i86Win32VS2010
                DESTINATION ${LIB_INSTALL_DIR}
                COMPONENT libraries_i86Win32VS2010
                )
            install(DIRECTORY ${PROJECT_BINARY_DIR}/../x64Win64VS2010/external-install/lib/x64Win64VS2010
                DESTINATION ${LIB_INSTALL_DIR}
                COMPONENT libraries_x64Win64VS2010
                )
            install(DIRECTORY ${PROJECT_BINARY_DIR}/../i86Win32VS2013/external-install/lib/i86Win32VS2013
                DESTINATION ${LIB_INSTALL_DIR}
                COMPONENT libraries_i86Win32VS2013
                )
            install(DIRECTORY ${PROJECT_BINARY_DIR}/../x64Win64VS2013/external-install/lib/x64Win64VS2013
                DESTINATION ${LIB_INSTALL_DIR}
                COMPONENT libraries_x64Win64VS2013
                )
        else()
            if(EPROSIMA_BUILD)
                install(DIRECTORY ${PROJECT_BINARY_DIR}/external-install/lib/${MSVC_ARCH}
                    DESTINATION ${LIB_INSTALL_DIR}
                    COMPONENT libraries_${MSVC_ARCH}
                    )
            else()
                install(DIRECTORY ${fastcdr_LIB_DIR}/${MSVC_ARCH}
                    DESTINATION ${LIB_INSTALL_DIR}
                    COMPONENT libraries_${MSVC_ARCH}
                    )
            endif()
        endif()

        # Install license
        if(EPROSIMA_BUILD)
            install(FILES ${PROJECT_BINARY_DIR}/external-install/FASTCDR_LIBRARY_LICENSE.txt
                DESTINATION .
                )
        else()
            # TODO Ver si puedo usar un LICENSE_INSTALL_DIR y que lo piye la configuración CMake.
        endif()
    endif()
endmacro()
