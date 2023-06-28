#!/bin/bash

files_to_exclude=(
    './include/fastrtps/types/*'
    )

files_needing_typeobject=(
    './examples/cpp/dds/ContentFilteredTopicExample/HelloWorld.idl'
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
    './include/fastdds/statistics/types.idl|../../../src/cpp/statistics/types'
    './include/fastrtps/monitor-service/ms_types.idl|../../../src/cpp/monitor-service/types'
    )

generated_files_needed_to_remove=(
    './src/cpp/monitor-service/types/types*'
)

# Temporal workaround for the include paths of generated files
# header include path - substitute
generated_files_needed_include_path_change=(
    './src/cpp/monitor-service/types/ms_types.h|__/__/fastdds/statistics/types.h|statistics/types/types.h'
    './src/cpp/monitor-service/types/ms_typesPubSubTypes.h|__/__/fastdds/statistics/typesPubSubTypes.h|statistics/types/typesPubSubTypes.h'
)

yellow='\E[1;33m'
textreset='\E[1;0m'

if [[ $(ls update_generated_code_from_idl.sh 2>/dev/null | wc -l) != 1 ]]; then
    echo "Please, execute this script from its directory"
    exit -1
fi

if [[ -z "$(which fastddsgen)" ]]; then
    echo "Cannot find fastddsgen. Please, include it in PATH environment variable"
    exit -1
fi

cd ../..

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

    # Detect if needs output directory.
    od_arg=""
    for od_entry in ${files_needing_output_dir[@]}; do
        if [[ $od_entry = $idl_file\|* ]]; then
            od_entry_split=(${od_entry//\|/ })
            od_arg="-d ${od_entry_split[1]}"
            break
        fi
    done

    fastddsgen -replace $to_arg $cs_arg $od_arg "$file_from_gen"

    if [[ $? != 0 ]]; then
        ret_value=-1
    fi

    cd -
done

# Change include paths if needed
generated_file=""
path_splits=""
for generated_file in ${generated_files_needed_include_path_change[@]}; do
    path_splits=(${generated_file//\|/ })
    sed -i "s#${path_splits[1]}#${path_splits[2]}#" ${path_splits[0]}
done

# Remove extra files
rm_file=""
for rm_file in ${generated_files_needed_to_remove[@]}; do
        rm $rm_file
    done

cd utils/scripts

exit $ret_value
