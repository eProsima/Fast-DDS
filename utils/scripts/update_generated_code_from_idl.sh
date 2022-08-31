#!/bin/bash

files_to_exclude=(
    './include/fastrtps/types/*'
    './include/fastdds/statistics/types.idl'
    './test/unittest/dynamic_types/idl/new_features_4_2.idl'
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

    fastddsgen -replace "$file_from_gen"

    if [[ $? != 0 ]]; then
        ret_value=-1
    fi

    cd -
done

cd utils/scripts

exit $ret_value
