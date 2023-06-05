# Run fastddsgen to create header/source from .idl
# Preserve subdirectory structure.
# If all idl are unchanged, then skip generation
# Generates type naming compatible with ROS 2.
#
# fastddsgen(
#     idl_root_path         # root directory for .idl files (.idl include directory)
#     idl_relative_path     # list of subfolder containing .idl files, or list of .idl files, relative to idl_root_path
#     output_path           # Directory for the generated files
#     generated_header      # List of generated header files (absolute path)
#     generated_source      # List of generated source files (absolute path)
#     generated_include_dir # List of include paths for compiling generated_source
# )
#
# This function runs fastddsgen using the following command:
#
#     fastddsgen -typeros2 -replace \
#                -d output_path \
#                -t CMAKE_CURRENT_BINARY_DIR/tmp/fastddsgen \
#                -I idl_root_path \
#                idl_root_path/idl_relative_path
#
# Example directory of input idl files:
#
#     /home/username/myproject/foo/msg/a.idl
#     /home/username/myproject/foo/msg/b.idl
#     /home/username/myproject/bar/msg/b.idl (contains #include <foo/msg/a.idl>)
#     /home/username/myproject/bar/msg/c.idl (contains #include <foo/msg/b.idl>)
#     /home/username/myproject/CMakeLists.txt
#
# Example function usage #1:
#
#     fastddsgen(
#         ${CMAKE_CURRENT_SOURCE_DIR}
#         "foo;bar"
#         ${CMAKE_CURRENT_SOURCE_DIR}/generated
#         myproject_idl_header
#         myproject_idl_source
#         myproject_idl_include_directories
# 
# Example function usage #2:
#
#     set(myproject_idl
#         foo/msg/a.idl
#         foo/msg/b.idl)
#         bar/msg/b.idl
#         bar/msg/c.idl)
#     fastddsgen(
#         ${CMAKE_CURRENT_SOURCE_DIR}
#         "${myproject_idl}"
#         ${CMAKE_CURRENT_SOURCE_DIR}/generated
#         myproject_idl_header
#         myproject_idl_source
#         myproject_idl_include_directories
# 
# Example output directory:
#
#     /home/username/myproject/build/generated/foo/msg/a.h,*.cxx
#     /home/username/myproject/build/generated/foo/msg/b.h,*.cxx
#     /home/username/myproject/build/generated/bar/msg/b.h,*.cxx
#     /home/username/myproject/build/generated/bar/msg/c.h,*.cxx
#
# Difference between fastddsgen.cmake and fastddsgen java executable:
# 
# * fastddsgen.cmake will not auto-generate dependency .idl (#include)
# * fastddsgen.cmake will generate idl with the same file name (foo/msg/b.idl, bar/msg/b.idl)
# * fastddsgen.cmake is much slower

function(fastddsgen 
    idl_root_path
    idl_relative_path
    output_path
    generated_header
    generated_source
    generated_include_directories
)
    # create a list of relative path to each idl
    # create a list of absolute path to each idl
    set(idl_rel_files)
    foreach(item ${idl_relative_path})
        if(IS_DIRECTORY "${idl_root_path}/${item}")
            # find all idl in subfolder, and append to list
            file(GLOB_RECURSE temp RELATIVE ${idl_root_path} ${idl_root_path}/${item}/*.idl)
            list(APPEND idl_rel_files ${temp})
        else()
            # verify that file exists
            if(NOT EXISTS "${idl_root_path}/${item}")
                message(FATAL_ERROR "file does not exist: ${idl_root_path}/${item}")
                continue()
            endif()

            # verify that file has .idl extension
            get_filename_component(ext ${item} EXT)
            if (NOT ${ext} STREQUAL ".idl")
                message(FATAL_ERROR "file is not idl: ${idl_root_path}/${item}")
                continue()
            endif()

            # append to list
            list(APPEND idl_rel_files ${item})
        endif()
    endforeach()
    # message(STATUS "idl rel paths: ${idl_rel_files}")

    # create list of subfolder for each idl
    # create list of filename (without extension) for each idl
    set(idl_subfolders)
    set(idl_filenames)
    foreach(idl ${idl_rel_files})
        get_filename_component(subfolder ${idl} DIRECTORY)
        get_filename_component(filename ${idl} NAME_WE)
        if( "${subfolder}" STREQUAL "" )
            set(subfolder ".")
        endif()
        list(APPEND idl_subfolders ${subfolder})
        list(APPEND idl_filenames ${filename})
    endforeach()
    # message(STATUS "idl subfolder: ${idl_subfolders}")
    # message(STATUS "idl filenames: ${idl_filenames}")

    # check if any of the generated files are missing or out-of-date
    set(generated_files_up_to_date TRUE)
    foreach(subfolder filename IN ZIP_LISTS idl_subfolders idl_filenames)

        # check if any of the generated files are missing
        if((NOT EXISTS "${output_path}/${subfolder}/${filename}.h") OR
           (NOT EXISTS "${output_path}/${subfolder}/${filename}.cxx") OR
           (NOT EXISTS "${output_path}/${subfolder}/${filename}PubSubTypes.h") OR
           (NOT EXISTS "${output_path}/${subfolder}/${filename}PubSubTypes.cxx"))
            # message(STATUS "generation required because file missing: ${output_path}/${subfolder}/${filename}.h,.cxx")
            fastddsgen_delete_generated("${output_path}/${subfolder}/${filename}")
            set(last_${output_path}_${subfolder}_${filename}_md5 "0" CACHE STRING "do not modify" FORCE)
            set(generated_files_up_to_date FALSE)
           continue()
        endif()

        # check if the .idl file has changed, by calculating the MD5
        # and comparing to a cmake cache variable called:
        # last_${idl_output_path}_${subfolder}_${filename}_md5
        file(MD5 "${idl_root_path}/${subfolder}/${filename}.idl" this_md5)
        if(NOT "${this_md5}" STREQUAL "${last_${output_path}_${subfolder}_${filename}_md5}")
            # message(STATUS "generation required because file out-of-date: ${idl_root_path}/${subfolder}/${filename}.idl")
            fastddsgen_delete_generated("${output_path}/${subfolder}/${filename}")
            set(last_${output_path}_${subfolder}_${filename}_md5 "0" CACHE STRING "do not modify" FORCE)
            set(generated_files_up_to_date FALSE)
            continue()
        endif()

    endforeach()

    if(${generated_files_up_to_date})
        # no change so skip fastddsgen
        # do not return, because we still need to set the output variables
        message(STATUS "fastddsgen skipping generation.\n"
            "   Generated files already exists in: ${output_path}\n"
            "   No changes to: ${idl_root_path}/\n"
            "                  ${idl_relative_path}")
    else()
        # find fastddsgen executable
        # "fastddsgen" for Unix systems, "fastddsgen.bat" for Windows
        find_program(FASTDDSGEN NAMES fastddsgen fastddsgen.bat REQUIRED)
        message(STATUS "Found fastddsgen: ${FASTDDSGEN}")

        # set tmp folder used by fastddsgen
        # fastddsgen uses /tmp by default, which can cause problems with user permissions.
        # Note: this tmp folder cannot be in the path of the generated files.
        set(tmp_fastddsgen_path ${CMAKE_CURRENT_BINARY_DIR}/tmp/fastddsgen/runtime)

        # set tmp folder where all the generated files will be created
        # the files will then be copied from the tmp folder into it's output subfolder
        set(tmp_output_path ${CMAKE_CURRENT_BINARY_DIR}/tmp/fastddsgen/output)

        message(STATUS "Running fastddsgen. Generate from: ${idl_root_path}\n"
                       "                    Generate to:   ${output_path}")
        foreach(subfolder filename IN ZIP_LISTS idl_subfolders idl_filenames)

            # create temporary folders
            execute_process(COMMAND mkdir -p ${tmp_fastddsgen_path})
            execute_process(COMMAND mkdir -p ${tmp_output_path})

            # absolute path to the idl
            get_filename_component(idl_abs_file "${idl_root_path}/${subfolder}/${filename}.idl" REALPATH)

            # run fastddsgen.
            # fastrtps must be run in idl_root_path or a parent of idl_root_path
            # to generate the proper include paths: #include "foo/msg/a.h"
            # instead of the include path:          #include "a.h"
            message(STATUS "    ${idl_abs_file}")
            execute_process(
                COMMAND ${FASTDDSGEN} -cs -typeros2 -replace -d ${tmp_output_path} -t ${tmp_fastddsgen_path} -I ${idl_root_path} ${idl_abs_file}
                WORKING_DIRECTORY ${idl_root_path}
                RESULT_VARIABLE fastrtps_result
                OUTPUT_VARIABLE fastrtps_output
                ERROR_VARIABLE fastrtps_error
                OUTPUT_STRIP_TRAILING_WHITESPACE)

            # check for error, and print fastddsgen output
            if(NOT "${fastrtps_result}" EQUAL "0")
                # Indent the output, because it is multi-line.
                string(REPLACE "\n" "\n   " fastrtps_output ${fastrtps_output})
                string(REPLACE "\n" "\n   " fastrtps_error ${fastrtps_error})

                message(STATUS "${fastrtps_output}")
                message(STATUS "${fastrtps_error}")
                message(FATAL_ERROR "fastrtspgen failed with return code: ${fastrtps_result}")
            endif()

            # if generation successful, then save (cache) the MD5 of each idl file
            # in a variable called "last_${output_path}_${subfolder}_${filename}_md5"
            # hide the variable from cmake-gui
            file(MD5 ${idl_abs_file} this_md5)
            set(last_${output_path}_${subfolder}_${filename}_md5 ${this_md5} CACHE STRING "do not modify" FORCE)
            mark_as_advanced(FORCE last_${output_path}_${subfolder}_${filename}_md5)

            # create output subfolders
            execute_process(COMMAND mkdir -p ${output_path}/${subfolder})

            # move the generated .h / .cxx to output_path/subfolder/
            # message(STATUS "moving ${output_path}/${filename}* to ${output_path}/${subfolder}/${filename}*")
            if (EXISTS ${tmp_output_path}/${filename}.h)
                file(RENAME ${tmp_output_path}/${filename}.h              ${output_path}/${subfolder}/${filename}.h)
            endif()
            if (EXISTS ${tmp_output_path}/${filename}.cxx)
                file(RENAME ${tmp_output_path}/${filename}.cxx            ${output_path}/${subfolder}/${filename}.cxx)
            endif()
            if(EXISTS ${tmp_output_path}/${filename}PubSubTypes.h)
                file(RENAME ${tmp_output_path}/${filename}PubSubTypes.h   ${output_path}/${subfolder}/${filename}PubSubTypes.h)
            endif()
            if(EXISTS ${tmp_output_path}/${filename}PubSubTypes.cxx)
                file(RENAME ${tmp_output_path}/${filename}PubSubTypes.cxx ${output_path}/${subfolder}/${filename}PubSubTypes.cxx)
            endif()

            # delete temporary folders
            execute_process(COMMAND rm -rf ${tmp_fastddsgen_path})
            execute_process(COMMAND rm -rf ${tmp_output_path})

        endforeach()
    endif()

    # for all generated files, add to the list of generated header/source
    set(generated_header_to_return)
    set(generated_source_to_return)
    set(generated_include_dirs_to_return ${output_path})
    foreach(subfolder filename IN ZIP_LISTS idl_subfolders idl_filenames)
        # search for the files, don't assume they were generated
        file(GLOB generated_h               ${output_path}/${subfolder}/${filename}.h)
        file(GLOB generated_cxx             ${output_path}/${subfolder}/${filename}.cxx)
        file(GLOB generated_PubSubTypes_h   ${output_path}/${subfolder}/${filename}PubSubTypes.h)
        file(GLOB generated_PubSubTypes_cxx ${output_path}/${subfolder}/${filename}PubSubTypes.cxx)

        # cmake function arguments are not normal variables. For simplicity:
        # 1. create local variable with return values
        # 2. set the function arguments = local variable variables at end of function
        list(APPEND generated_header_to_return ${generated_h}   ${generated_PubSubTypes_h})
        list(APPEND generated_source_to_return ${generated_cxx} ${generated_PubSubTypes_cxx})

        list(APPEND generated_include_dirs_to_return "${output_path}/${subfolder}")
        list(REMOVE_DUPLICATES generated_include_dirs_to_return)
    endforeach()

    # mark as generated
    set_source_files_properties(${generated_header_to_return} ${generated_source_to_return} PROPERTIES GENERATED TRUE)

    # message(STATUS "generated_header: ${generated_header_to_return}")
    # message(STATUS "generated_source: ${generated_source_to_return}")
    # message(STATUS "generated_include_directories: ${generated_include_dirs_to_return}")

    # copy result to function arguments
    set(${generated_header}              ${generated_header_to_return}       PARENT_SCOPE)
    set(${generated_source}              ${generated_source_to_return}       PARENT_SCOPE)
    set(${generated_include_directories} ${generated_include_dirs_to_return} PARENT_SCOPE)
endfunction()

function(fastddsgen_delete_generated output_file_without_ext)
    file(REMOVE "${output_file_without_ext}.h")
    file(REMOVE "${output_file_without_ext}.cxx")
    file(REMOVE "${output_file_without_ext}PubSubTypes.h")
    file(REMOVE "${output_file_without_ext}PubSubTypes.cxx")
endfunction()
