# returns true if only a single one of its arguments is true
function(xor result)
    set(true_args_count 0)

    foreach(foo ${ARGN})
        if(foo)
            math(EXPR true_args_count "${true_args_count}+1")
        endif()
    endforeach()

    if(NOT (${true_args_count} EQUAL 1))
        set(${result} FALSE PARENT_SCOPE)
    else()
        set(${result} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(at_most_one result)
    set(true_args_count 0)

    foreach(foo ${ARGN})
        if(foo)
            math(EXPR true_args_count "${true_args_count}+1")
        endif()
    endforeach()

    if(${true_args_count} GREATER 1)
        set(${result} FALSE PARENT_SCOPE)
    else()
        set(${result} TRUE PARENT_SCOPE)
    endif()
endfunction()
