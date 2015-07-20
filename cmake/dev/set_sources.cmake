macro(set_sources)
    set(${PROJECT_NAME}_SOURCES
        ${ARGN}
        )

    set_property(GLOBAL PROPERTY ${PROJECT_NAME}_SOURCES_PROPERTY ${${PROJECT_NAME}_SOURCES})
endmacro()

macro(set_public_headers_directory abs_directory rel_directory)

    install(DIRECTORY ${abs_directory}/${rel_directory}
        ${ARGN}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        )

    get_property(${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES_PROPERTY)
    set(${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES ${${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES} ${abs_directory})
    set_property(GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES_PROPERTY ${${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES})

endmacro()

macro(set_public_header directory file)
    install(FILES ${directory}/${file}
        ${ARGN}
        )

    get_property(${PROJECT_NAME}_PUBLIC_HEADERS_FILES GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_FILES_PROPERTY)
    set(${PROJECT_NAME}_PUBLIC_HEADERS_FILES ${${PROJECT_NAME}_PUBLIC_HEADERS_FILES} ${file})
    set_property(GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_FILES_PROPERTY ${${PROJECT_NAME}_PUBLIC_HEADERS_FILES})
endmacro()
