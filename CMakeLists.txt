###############################################################################
# CMake build rules for FastRTPS                                                
###############################################################################
cmake_minimum_required(VERSION 2.8.12)

# Set CMAKE_BUILD_TYPE to Release by default.
if(DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        )
else()
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        )
endif()

project("fastrtps")
set(PROJECT_NAME_STYLED "FastRTPS")
set(PROJECT_NAME_LARGE "Fast RTPS")
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)
set(${PROJECT_NAME}_DESCRIPTION_SUMMARY "C++ library implementation of RTPS protocol")
set(${PROJECT_NAME}_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} library provides publication/subscription communication using RTPS protocol.")

message(STATUS "Configuring ${PROJECT_NAME_LAGE}")

###############################################################################
# Version information                                                         #
###############################################################################

#If not set, copy over product version from configure.ac
if(NOT PROJECT_MAJOR_VERSION)
    file(STRINGS
        ${PROJECT_SOURCE_DIR}/configure.ac
        PROJECT_MAJOR_VERSION_TMP
        REGEX "^m4_define\\(\\[?version_major\\]?, *\\[?[0-9]+\\]?\\)"
        )
    file(STRINGS
        ${PROJECT_SOURCE_DIR}/configure.ac
        PROJECT_MINOR_VERSION_TMP
        REGEX "^m4_define\\(\\[?version_minor\\]?, *\\[?[0-9]+\\]?\\)"
        )
    file(STRINGS
        ${PROJECT_SOURCE_DIR}/configure.ac
        PROJECT_MICRO_VERSION_TMP
        REGEX "^m4_define\\(\\[?version_micro\\]?, *\\[?[0-9]+\\]?\\)"
        )

    string(REGEX REPLACE "m4_define\\(\\[?version_major\\]?, *\\[?([0-9]+)\\]?\\)"
        "\\1"
        PROJECT_MAJOR_VERSION
        ${PROJECT_MAJOR_VERSION_TMP}
        )
    string(REGEX REPLACE "m4_define\\(\\[?version_minor\\]?, *\\[?([0-9]+)\\]?\\)"
        "\\1"
        PROJECT_MINOR_VERSION
        ${PROJECT_MINOR_VERSION_TMP}
        )
    string(REGEX REPLACE "m4_define\\(\\[?version_micro\\]?, *\\[?([0-9]+)\\]?\\)"
        "\\1"
        PROJECT_MICRO_VERSION
        ${PROJECT_MICRO_VERSION_TMP}
        )
endif()

set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_MICRO_VERSION})

message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "To change de version modify the file configure.ac")

###############################################################################
# Generation of windows installers.
###############################################################################
if(WIN32)
    option(EPROSIMA_INSTALLER "Activate the creation of a build to create windows installers" OFF)
endif()

###############################################################################
# Warning level
###############################################################################
if(MSVC OR MSVC_IDE)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS] /W4")
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wno-unknown-pragmas")
endif()

###############################################################################
# Test system configuration
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/dev/check_configuration.cmake)
check_stdcxx()
check_compile_feature()
check_endianness()

###############################################################################
# Check MSVC architecture
###############################################################################
if(MSVC OR MSVC_IDE)
    check_msvc_arch()
endif()

###############################################################################
# Installation paths
###############################################################################
set(BIN_INSTALL_DIR bin/ CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR include/ CACHE PATH "Installation directory for C++ headers")
set(LIB_INSTALL_DIR lib/ CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR share/ CACHE PATH "Installation directory for data")
if(WIN32)
    set(DOC_DIR "doc")
else()
    set(DOC_DIR "${DATA_INSTALL_DIR}/doc")
endif()
set(DOC_INSTALL_DIR ${DOC_DIR} CACHE PATH "Installation directory for documentation")
if(WIN32)
    set(LICENSE_INSTALL_DIR . CACHE PATH "Installation directory for licenses")
else()
    set(LICENSE_INSTALL_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME} CACHE PATH "Installation directory for licenses")
endif()

###############################################################################
# eProsima build options
###############################################################################
option(EPROSIMA_BUILD "Activate internal building" OFF)
option(MINION "Tells if it is a subproject of a superproject" OFF)

if(EPROSIMA_INSTALLER)
    set(EPROSIMA_BUILD ON)
endif()

###############################################################################
# Check libraries
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/dev/boost_libraries.cmake)
check_boost(thread system date_time chrono regex atomic)

###############################################################################
# Load external eProsima projects.
###############################################################################
option(THIRDPARTY "Activate the use of internal thirdparties" OFF)

if(EPROSIMA_BUILD)
    set(THIRDPARTY ON)
endif()

include(${PROJECT_SOURCE_DIR}/cmake/dev/eprosima_libraries.cmake)

find_eprosima_package(fastcdr)

###############################################################################
# Java application
###############################################################################
option(BUILD_JAVA "Activate the compilation of the Java application" OFF)

if(EPROSIMA_INSTALLER)
    set(BUILD_JAVA ON)
endif()

if(BUILD_JAVA)
    include(${PROJECT_SOURCE_DIR}/cmake/dev/java_support.cmake)

    gradle_build(${PROJECT_SOURCE_DIR}/${PROJECT_NAME}gen
        THIRDPARTY_DEPENDENCY idl
        THIRDPARTY_DEPENDENCY fastcdr
        )
endif()

###############################################################################
# Compile library.
###############################################################################
add_subdirectory(src/cpp)

###############################################################################
# Testing
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/dev/gtest.cmake)
check_gtest()
if(GTEST_FOUND)
    enable_testing()
    include(CTest)
    add_subdirectory(test)
endif()

###############################################################################
# Documentation
###############################################################################
# Add an option to toggle the generation of the API documentation.
option(BUILD_DOCUMENTATION "Use doxygen and libreoffice to create product documentation" OFF)

if(EPROSIMA_INSTALLER)
    set(BUILD_DOCUMENTATION ON)
    set(DOXYGEN_SHOULD_SKIP_THIS_PUBLIC "DOXYGEN_SHOULD_SKIP_THIS_PUBLIC")
endif()

if(BUILD_DOCUMENTATION)
    find_package(Doxygen)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "doxygen is needed to build the documentation. Please install it correctly")
    endif()
    find_package(LATEX)
    if(NOT LATEX_COMPILER)
        message(FATAL_ERROR "latex is needed to build the documentation. Please install it correctly")
    endif()
    if(UNIX)
        find_program(DOXYFILE_MAKE make)
        if(DOXYFILE_MAKE)
            message(STATUS "Found Make: ${DOXYFILE_MAKE}")
        else()
            message(FATAL_ERROR "make is needed to build the documentation. Please install it correctly")
        endif()
    elseif(WIN32)
        set(DOXYFILE_MAKE make.bat)
    endif()
    find_program(LIBREOFFICE_EXE soffice)
    if(LIBREOFFICE_EXE)
        message(STATUS "Found Libreoffice: ${LIBREOFFICE_EXE}")
        message(STATUS "Recall you need to have installed the eProsima macros for Libreoffice")
    else()
        message(FATAL_ERROR "libreoffice is needed to build the documentation. Please install it correctly")
    endif()
    
    # Target to create documentation directories
    add_custom_target(docdirs
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/doc
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/doc/pdf
        COMMENT "Creating documentation directory" VERBATIM)

    ### Doxygen ########################3

    # Configure the template doxyfile for or specific project
    configure_file(doxyfile.in ${PROJECT_BINARY_DIR}/doxyfile @ONLY IMMEDIATE)
    # Add custom target to run doxygen when ever the project is build
    add_custom_target(doxygen
        COMMAND "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/doxyfile"
        SOURCES "${PROJECT_BINARY_DIR}/doxyfile"
        COMMENT "Generating API documentation with doxygen" VERBATIM)
    # Add custom post_build target to run make and generate latex
    # documentation.
    add_custom_command(TARGET doxygen POST_BUILD
        COMMAND "${DOXYFILE_MAKE}"
        COMMAND cp refman.pdf "${PROJECT_BINARY_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - C++ API Manual.pdf"
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/doxygen/latex"
        COMMENT "Running LaTex for doxygen documentation" VERBATIM)

    add_dependencies(doxygen docdirs)

    # Instalation of doxygen files
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html
        DESTINATION ${DOC_INSTALL_DIR}
        COMPONENT documentation
        )

    install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - C++ API Manual.pdf"
        DESTINATION ${DOC_INSTALL_DIR}/pdf
        COMPONENT documentation
        )

    ### Libreoffice ########################3

    if(WIN32)
        set(LIBREOFFICE_CONVERT_README_MACRO "macro:///eProsima.documentation.changeHyperlinksAndVersionToHTML(${PROJECT_SOURCE_DIR}/doc/README.odt,${PROJECT_VERSION},./,/doc/)")
        set(README_LOCATION "${PROJECT_BINARY_DIR}/")
        set(README_INSTALL_LOCATION ".")
    else()
        set(LIBREOFFICE_CONVERT_README_MACRO "macro:///eProsima.documentation.changeVersionToHTML(${PROJECT_SOURCE_DIR}/doc/README.odt,${PROJECT_VERSION})")
        set(README_LOCATION "${PROJECT_BINARY_DIR}/doc/")
        set(README_INSTALL_LOCATION "${DOC_INSTALL_DIR}")
    endif()

    add_custom_target(libreoffice
        COMMAND "${LIBREOFFICE_EXE}" --headless
        "macro:///eProsima.documentation.changeVersion(${PROJECT_SOURCE_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - Installation Manual.odt,${PROJECT_VERSION})"
        COMMAND ${CMAKE_COMMAND} -E rename "${PROJECT_SOURCE_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - Installation Manual.pdf" "${PROJECT_BINARY_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - Installation Manual.pdf"
        COMMAND "${LIBREOFFICE_EXE}" --headless
        "macro:///eProsima.documentation.changeVersion(${PROJECT_SOURCE_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - User Manual.odt,${PROJECT_VERSION})"
        COMMAND ${CMAKE_COMMAND} -E rename "${PROJECT_SOURCE_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - User Manual.pdf" "${PROJECT_BINARY_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - User Manual.pdf"
        COMMAND "${LIBREOFFICE_EXE}" --headless
        "macro:///eProsima.documentation.changeVersion(${PROJECT_SOURCE_DIR}/doc/pdf/${PROJECT_NAME_UPPER}GEN - User Manual.odt,${PROJECT_VERSION})"
        COMMAND ${CMAKE_COMMAND} -E rename "${PROJECT_SOURCE_DIR}/doc/pdf/${PROJECT_NAME_UPPER}GEN - User Manual.pdf" "${PROJECT_BINARY_DIR}/doc/pdf/${PROJECT_NAME_UPPER}GEN - User Manual.pdf"
        COMMAND "${LIBREOFFICE_EXE}" --headless ${LIBREOFFICE_CONVERT_README_MACRO} 
        COMMAND ${CMAKE_COMMAND} -E rename "${PROJECT_SOURCE_DIR}/doc/README.html" "${README_LOCATION}/README.html"
        DEPENDS
        "${PROJECT_SOURCE_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - Installation Manual.odt"
        "${PROJECT_SOURCE_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - User Manual.odt"
        "${PROJECT_SOURCE_DIR}/doc/pdf/${PROJECT_NAME_UPPER}GEN - User Manual.odt"
        "${PROJECT_SOURCE_DIR}/doc/README.odt"
        COMMENT "Generating documentation with libreoffice" VERBATIM)

    add_dependencies(libreoffice docdirs)

    add_custom_target(doc ALL
        COMMENT "Generated project documentation" VERBATIM)

    add_dependencies(doc doxygen libreoffice)
endif()

###############################################################################
# Packaging
###############################################################################
# Install external eprosima libraries
install_eprosima_libraries()
install_boost(ALL thread system date_time chrono regex atomic)

# Install licenses
install(FILES ${PROJECT_SOURCE_DIR}/${PROJECT_NAME_UPPER}_LIBRARY_LICENSE.txt
    ${PROJECT_SOURCE_DIR}/LGPLv3_LICENSE.txt
    DESTINATION ${LICENSE_INSTALL_DIR}
    COMPONENT licenses
    )
set(CPACK_COMPONENT_LICENSES_HIDDEN 1)

# Install examples
install(DIRECTORY ${PROJECT_SOURCE_DIR}/examples
    DESTINATION .
    COMPONENT examples
    )

set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} examples")

set(CPACK_COMPONENTS_ALL headers licenses examples)

if(BUILD_JAVA)
    install(FILES ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}gen/share/fastrtps/${PROJECT_NAME}gen.jar
        DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}
        COMPONENT java
        )

    if(WIN32)
        install(PROGRAMS
            ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}gen/scripts/${PROJECT_NAME}gen.bat
            DESTINATION ${BIN_INSTALL_DIR}
            COMPONENT java
            )
    else()
        install(PROGRAMS
            ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}gen/scripts/${PROJECT_NAME}gen
            DESTINATION ${BIN_INSTALL_DIR}
            COMPONENT java
            )
    endif()

    ### Java component
    set(CPACK_COMPONENT_JAVA_DISPLAY_NAME "Java application")
    set(CPACK_COMPONENT_JAVA_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} generator")

    set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} java)
endif()

if(BUILD_DOCUMENTATION)
    # Instalation of pdf files
    install(FILES 
        "${PROJECT_BINARY_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - Installation Manual.pdf"
        "${PROJECT_BINARY_DIR}/doc/pdf/${PROJECT_NAME_LARGE} - User Manual.pdf"
        "${PROJECT_BINARY_DIR}/doc/pdf/${PROJECT_NAME_UPPER}GEN - User Manual.pdf"
        DESTINATION ${DOC_INSTALL_DIR}/pdf
        COMPONENT documentation
        )

    install(FILES 
        "${README_LOCATION}/README.html"
        DESTINATION ${README_INSTALL_LOCATION}
        COMPONENT documentation
        )

    ### Documentation component
    set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")
    set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} documentation")

    set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} documentation)
endif()

if(MSVC OR MSVC_IDE)
    if(EPROSIMA_INSTALLER)
        set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL}
            libraries_i86Win32VS2010
            libraries_x64Win64VS2010
            libraries_i86Win32VS2013
            libraries_x64Win64VS2013
            )
    else()
        set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} libraries_${MSVC_ARCH})
    endif()
else()
    set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} libraries)
endif()

include(${PROJECT_SOURCE_DIR}/cmake/packaging/eProsimaPackaging.cmake)
