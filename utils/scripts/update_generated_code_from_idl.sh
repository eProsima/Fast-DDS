#!/usr/bin/env bash

files_to_exclude=(
    './include/fastrtps/types/*'
    )

files_needing_typeobject=(
    './examples/cpp/dds/ContentFilteredTopicExample/HelloWorld.idl'
    './test/blackbox/types/HelloWorld.idl'
    './test/blackbox/types/TestIncludeRegression3361.idl'
    './test/blackbox/types/TestRegression3361.idl'
    './test/unittest/dds/topic/DDSSQLFilter/data_types/ContentFilterTestType.idl'
    './test/unittest/dynamic_types/idl/Basic.idl'
    './test/unittest/dynamic_types/idl/new_features_4_2.idl'
    './test/unittest/dynamic_types/idl/Test.idl'
    './test/unittest/xtypes/idl/Types.idl'
    './test/unittest/xtypes/idl/WideEnum.idl'
    './test/xtypes/idl/Types.idl'
    )

files_needing_case_sensitive=(
    './test/unittest/dynamic_types/idl/new_features_4_2.idl'
    )

files_needing_output_dir=(
    './include/fastdds/statistics/types.idl|../../../src/cpp/statistics/types|../../../test/blackbox/types/statistics'
    './include/fastdds/statistics/monitorservice_types.idl|../../../src/cpp/statistics/types'
    './test/blackbox/types/core/core_types.idl|.'
    )

red='\E[1;31m'
yellow='\E[1;33m'
textreset='\E[1;0m'

current_dir=$(git rev-parse --show-toplevel)

if [[ ! "$(pwd -P)" -ef "$current_dir" ]]; then
    echo -e "${red}This script must be executed in the repository root directory.${textreset}"
    exit -1
fi

if [[ -z "$(which fastddsgen)" ]]; then
    echo "Cannot find fastddsgen. Please, include it in PATH environment variable"
    exit -1
fi

readarray -d '' idl_files < <(find . -iname \*.idl -print0)

for del in ${files_to_exclude[@]}
do
    idl_files=("${idl_files[@]/$del}")
done

idl_files=(${idl_files[@]/$files_to_exclude})

ret_value=0

for idl_file in "${idl_files[@]}"; do
    idl_dir=$(dirname "$idl_file")
    file_from_gen=$(basename "$idl_file")

    echo -e "Processing ${yellow}$idl_file${textreset}"

    cd "${idl_dir}"

    # Detect if needs type_object.
    [[ ${files_needing_typeobject[*]} =~ $idl_file ]] && to_arg='-typeobject' || to_arg=''

    # Detect if needs case sensitive.
    [[ ${files_needing_case_sensitive[*]} =~ $idl_file ]] && cs_arg='-cs' || cs_arg=''

    # Detect if needs output directories.
    not_processed=true
    for od_entry in ${files_needing_output_dir[@]}; do
        if [[ $od_entry = $idl_file\|* ]]; then
            not_processed=false;
            od_entry_split=(${od_entry//\|/ })
            for od_entry_split_element in ${od_entry_split[@]:1}; do
                od_arg="-d ${od_entry_split_element}"
                fastddsgen -cdr both -replace $to_arg $cs_arg $od_arg "$file_from_gen"
            done
            break
        fi
    done

    if $not_processed ; then
        fastddsgen -cdr both -replace $to_arg $cs_arg "$file_from_gen"
    fi

    if [[ $? != 0 ]]; then
        ret_value=-1
    fi

    cd -
done

exit $ret_value
