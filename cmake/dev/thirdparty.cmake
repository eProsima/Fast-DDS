macro(check_thirdparties)
    execute_process(COMMAND git submodule update --recursive --init
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
endmacro()
