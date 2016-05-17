macro(check_boost)
    if(WIN32)
        option(EPROSIMA_BOOST "Activate special set of BOOST_LIBRARYDIR" OFF)
        if(EPROSIMA_BUILD)
            set(EPROSIMA_BOOST ON)
        endif()
    endif()

    # Find package Boost
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
    if(WIN32 AND EPROSIMA_BOOST)
        set(BOOST_LIBRARYDIR_ $ENV{BOOST_LIBRARYDIR})
        if(BOOST_LIBRARYDIR_)
            file(TO_CMAKE_PATH "${BOOST_LIBRARYDIR_}/${MSVC_ARCH}" BOOST_LIBRARYDIR)
        endif()
    endif()
    find_package(Boost REQUIRED COMPONENTS ${ARGN})
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Cannot find Boost components: ${ARGN}")
    endif()
endmacro()

macro(install_boost FILETYPE)
    if(MSVC OR MSVC_IDE)
        set(RUNTIME_FILES_ 0)
        set(LIBRARY_FILES_ 0)
        if("${FILETYPE}" MATCHES "^([Rr][Uu][Nn][Tt][Ii][Mm][Ee])$")
            set(RUNTIME_FILES_ 1)
        elseif("${FILETYPE}" MATCHES "^([Ll][Ii][Bb][Rr][Aa][Rr][Yy])$")
            set(LIBRARY_FILES_ 1)
        elseif("${FILETYPE}" MATCHES "^([Aa][Ll][Ll])$")
            set(RUNTIME_FILES_ 1)
            set(LIBRARY_FILES_ 1)
        else()
            message(FATAL_ERROR "Bad parameter in install_boost macro")
        endif()

        foreach(arg_ ${ARGN})
            if(EPROSIMA_BUILD AND NOT EPROSIMA_INSTALLER)
                if(MSVC10)
                    set(BOOST_ARCH "vc100")
                elseif(MSVC11)
                    set(BOOST_ARCH "vc110")
                elseif(MSVC12)
                    set(BOOST_ARCH "vc120")
                elseif(MSVC14)
                    set(BOOST_ARCH "vc140")
                endif()

                set(DIR_EXTENSION "")
                if(EPROSIMA_INSTALLER_MINION)
                    set(DIR_EXTENSION "/${MSVC_ARCH}")
                endif()

                #Normalize path
                get_filename_component(BOOST_LIBRARYDIR_NORMALIZE ${BOOST_LIBRARYDIR} ABSOLUTE)

                # Runtime
                if(RUNTIME_FILES_)
                    install(DIRECTORY ${BOOST_LIBRARYDIR_NORMALIZE}/
                        DESTINATION ${BIN_INSTALL_DIR}${DIR_EXTENSION}
                        COMPONENT libraries_${MSVC_ARCH}
                        CONFIGURATIONS Debug
                        FILES_MATCHING
                        PATTERN "boost_${arg_}-${BOOST_ARCH}-mt-gd*.dll"
                        )

                    install(DIRECTORY ${BOOST_LIBRARYDIR_NORMALIZE}/
                        DESTINATION ${BIN_INSTALL_DIR}${DIR_EXTENSION}
                        COMPONENT libraries_${MSVC_ARCH}
                        CONFIGURATIONS Release
                        FILES_MATCHING
                        PATTERN "boost_${arg_}-${BOOST_ARCH}-mt*.dll"
                        PATTERN "boost_${arg_}-${BOOST_ARCH}-mt-gd*.dll" EXCLUDE
                        )
                endif()

                # Library
                if(LIBRARY_FILES_)
                    install(DIRECTORY ${BOOST_LIBRARYDIR_NORMALIZE}/
                        DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}
                        COMPONENT libraries_${MSVC_ARCH}
                        CONFIGURATIONS Debug
                        FILES_MATCHING
                        PATTERN "boost_${arg_}-${BOOST_ARCH}-mt-gd*.lib"
                        )

                    install(DIRECTORY ${BOOST_LIBRARYDIR_NORMALIZE}/
                        DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}
                        COMPONENT libraries_${MSVC_ARCH}
                        CONFIGURATIONS Release
                        FILES_MATCHING
                        PATTERN "boost_${arg_}-${BOOST_ARCH}-mt*.lib"
                        PATTERN "boost_${arg_}-${BOOST_ARCH}-mt-gd*.lib" EXCLUDE
                        )
                endif()
            endif()
        endforeach()
    endif()
endmacro()
