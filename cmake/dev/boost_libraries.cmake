macro(check_boost)
    # Find package Boost
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
    set(BOOST_ALL_DYN_LINK ON)
    if(WIN32 AND EPROSIMA_BUILD)
        set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR}/${MSVC_ARCH})
    endif()
    find_package(Boost COMPONENTS ${ARGN})
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Cannot find Boost libraries")
    endif()
endmacro()

macro(install_boost)
    if(MSVC OR MSVC_IDE)
        foreach(arg ${ARGN})
            if(EPROSIMA_INSTALLER)
                #i86Win32VS2010
                get_filename_component(BOOST_LIBRARYDIR_NORMALIZE "$ENV{BOOST_LIBRARYDIR}/i86Win32VS2010" ABSOLUTE)
                install(DIRECTORY ${BOOST_LIBRARYDIR_NORMALIZE}
                    DESTINATION lib
                    COMPONENT libraries_i86Win32VS2010
                    FILES_MATCHING
                    PATTERN "boost_${arg}-vc100-mt*"
                    )

                #x64Win64VS2010
                get_filename_component(BOOST_LIBRARYDIR_NORMALIZE "$ENV{BOOST_LIBRARYDIR}/x64Win64VS2010" ABSOLUTE)
                install(DIRECTORY ${BOOST_LIBRARYDIR_NORMALIZE}
                    DESTINATION lib
                    COMPONENT libraries_x64Win64VS2010
                    FILES_MATCHING
                    PATTERN "boost_${arg}-vc100-mt*"
                    )

                #i86Win32VS2013
                get_filename_component(BOOST_LIBRARYDIR_NORMALIZE "$ENV{BOOST_LIBRARYDIR}/i86Win32VS2013" ABSOLUTE)
                install(DIRECTORY ${BOOST_LIBRARYDIR_NORMALIZE}
                    DESTINATION lib
                    COMPONENT libraries_i86Win32VS2013
                    FILES_MATCHING
                    PATTERN "boost_${arg}-vc120-mt*"
                    )

                #x64Win64VS2013
                get_filename_component(BOOST_LIBRARYDIR_NORMALIZE "$ENV{BOOST_LIBRARYDIR}/x64Win64VS2013" ABSOLUTE)
                install(DIRECTORY ${BOOST_LIBRARYDIR_NORMALIZE}
                    DESTINATION lib
                    COMPONENT libraries_x64Win64VS2013
                    FILES_MATCHING
                    PATTERN "boost_${arg}-vc120-mt*"
                    )
            elseif(EPROSIMA_BUILD)
                if(MSVC10)
                    set(BOOST_ARCH "vc100")
                elseif(MSVC11)
                    set(BOOST_ARCH "vc110")
                elseif(MSVC12)
                    set(BOOST_ARCH "vc120")
                endif()

                #Normalize path
                get_filename_component(BOOST_LIBRARYDIR_NORMALIZE ${BOOST_LIBRARYDIR} ABSOLUTE)

                install(DIRECTORY ${BOOST_LIBRARYDIR_NORMALIZE}
                    DESTINATION lib
                    COMPONENT libraries_${MSVC_ARCH}
                    FILES_MATCHING
                    PATTERN "boost_${arg}-${BOOST_ARCH}-mt*"
                    )
            endif()
        endforeach()
    endif()
endmacro()
