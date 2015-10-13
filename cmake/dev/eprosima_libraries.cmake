macro(find_eprosima_package package)
    if(THIRDPARTY)
        set(USE_BOOST_ "")
        foreach(arg ${ARGN})
            if("${arg}" STREQUAL "USE_BOOST")
                set(USE_BOOST_ "-DEPROSIMA_BOOST=${EPROSIMA_BOOST}")
            endif()
        endforeach()

        set(${package}ExternalDir ${PROJECT_BINARY_DIR}/external/${package})
        if(NOT MSVC AND NOT MSVC_IDE)
            set(BUILD_OPTION "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
        endif()

        if(MINION)
            set(CMAKE_INSTALL_PREFIX_ "${CMAKE_INSTALL_PREFIX}")
        else()
            set(CMAKE_INSTALL_PREFIX_ "${PROJECT_BINARY_DIR}/external/install")
        endif()

        # Separate CMAKE_PREFIX_PATH
        string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ "${CMAKE_PREFIX_PATH}")
        set(${package}_CMAKE_ARGS
             "${PROJECT_SOURCE_DIR}/thirdparty/${package}"
             "-G \"${CMAKE_GENERATOR}\""
             ${BUILD_OPTION}
             ${USE_BOOST_}
             "-DMINION=ON"
             "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX_}"
             "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_}"
             )
        list(APPEND ${package}_CMAKE_ARGS LIST_SEPARATOR "|")

        file(MAKE_DIRECTORY ${${package}ExternalDir})
        file(WRITE ${${package}ExternalDir}/CMakeLists.txt
            "cmake_minimum_required(VERSION 2.8.11)\n"
            "include(ExternalProject)\n"
            "ExternalProject_Add(${package}\n"
             "CONFIGURE_COMMAND \"${CMAKE_COMMAND}\"\n"
             "${${package}_CMAKE_ARGS}\n"
             "DOWNLOAD_COMMAND echo\n"
             "UPDATE_COMMAND cd ${PROJECT_SOURCE_DIR} && git submodule update --recursive --init thirdparty/${package}\n"
             "SOURCE_DIR ${PROJECT_SOURCE_DIR}/thirdparty/${package}\n"
             "BINARY_DIR ${${package}ExternalDir}/build\n"
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
             execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
                 WORKING_DIRECTORY ${${package}ExternalDir}
                 RESULT_VARIABLE EXECUTE_RESULT
                 )

             if(NOT EXECUTE_RESULT EQUAL 0)
                 message(FATAL_ERROR "Cannot build Git submodule ${package} in debug mode")
             endif()

             execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
                 WORKING_DIRECTORY ${${package}ExternalDir}
                 RESULT_VARIABLE EXECUTE_RESULT
                 )

             if(NOT EXECUTE_RESULT EQUAL 0)
                 message(FATAL_ERROR "Cannot build Git submodule ${package} in release mode")
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

    find_package(${package})

endmacro()

macro(install_eprosima_libraries)
    if(MSVC OR MSVC_IDE)
        # Install includes
        install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/include/
            DESTINATION ${INCLUDE_INSTALL_DIR}
            COMPONENT headers
            )

        install(DIRECTORY ${PROJECT_BINARY_DIR}/external/install/lib/
            DESTINATION lib
            COMPONENT libraries_${MSVC_ARCH}
            )
    endif()
endmacro()
