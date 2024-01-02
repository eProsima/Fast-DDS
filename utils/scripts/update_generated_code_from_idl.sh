#!/usr/bin/env bash

files_to_exclude=(
    './include/fastrtps/types/*'
    )

files_not_needing_typeobject=(
    './include/fastdds/dds/xtypes/type_representation/detail/dds-xtypes_typeobject.idl'
    './src/cpp/fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.idl'
    './src/cpp/fastdds/builtin/type_lookup_service/detail/rpc_types.idl'
    )

files_needing_case_sensitive=(
    './test/unittest/dynamic_types/idl/new_features_4_2.idl'
    )

files_needing_output_dir=(
    './include/fastdds/statistics/types.idl|../../../src/cpp/statistics/types|../../../test/blackbox/types/statistics'
    './include/fastdds/statistics/monitorservice_types.idl|../../../src/cpp/statistics/types'
    )

files_needing_no_typesupport=(
    './include/fastdds/dds/core/detail/DDSReturnCode.idl'
    './include/fastdds/dds/core/detail/DDSSecurityReturnCode.idl'
    './include/fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.idl'
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
    [[ ${files_not_needing_typeobject[*]} =~ $idl_file ]] && to_arg='-no-typeobjectsupport' || to_arg=''

    # Detect if needs case sensitive.
    [[ ${files_needing_case_sensitive[*]} =~ $idl_file ]] && cs_arg='-cs' || cs_arg=''

    [[ ${files_needing_no_typesupport[*]} =~ $idl_file ]] && nosupport_arg='-no-typesupport' || nosupport_arg=''

    # Detect if needs output directories.
    not_processed=true
    for od_entry in ${files_needing_output_dir[@]}; do
        if [[ $od_entry = $idl_file\|* ]]; then
            not_processed=false;
            od_entry_split=(${od_entry//\|/ })
            for od_entry_split_element in ${od_entry_split[@]:1}; do
                od_arg="-d ${od_entry_split_element}"
                fastddsgen -replace -genapi $to_arg $cs_arg $od_arg "$file_from_gen" -no-dependencies
            done
            break
        fi
    done

    if $not_processed ; then
        fastddsgen -replace -genapi $to_arg $cs_arg $nosupport_arg "$file_from_gen" -no-dependencies
    fi

    if [[ $? != 0 ]]; then
        ret_value=-1
    fi

    cd -
done

# Move source files to src/cpp
mv ./include/fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobjectCdrAux.ipp ./src/cpp/fastdds/xtypes/type_representation/dds_xtypes_typeobjectCdrAux.ipp
mv ./include/fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobjectPubSubTypes.cxx ./src/cpp/fastdds/xtypes/type_representation/dds_xtypes_typeobjectPubSubTypes.cxx

sed -i 's+"dds_xtypes_typeobjectCdrAux.hpp"+<fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobjectCdrAux.hpp>+' ./src/cpp/fastdds/xtypes/type_representation/dds_xtypes_typeobjectCdrAux.ipp
sed -i 's+"dds_xtypes_typeobjectCdrAux.hpp"+<fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobjectCdrAux.hpp>+' ./src/cpp/fastdds/xtypes/type_representation/dds_xtypes_typeobjectPubSubTypes.cxx
sed -i 's+"dds_xtypes_typeobjectPubSubTypes.h"+<fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobjectPubSubTypes.h>+' ./src/cpp/fastdds/xtypes/type_representation/dds_xtypes_typeobjectPubSubTypes.cxx

sed -i 's+"../../../../../../include/fastdds/dds/xtypes/type_representation/detail/dds-xtypes_typeobject.hpp"+<fastdds/dds/xtypes/type_representation/TypeObject.hpp>+' ./src/cpp/fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.hpp
sed -i 's+"../../../../../../include/fastdds/dds/core/detail/DDSReturnCode.hpp"+<fastdds/dds/core/ReturnCode.hpp>+' ./src/cpp/fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.hpp

sed -i 's+"../../../../../../include/fastdds/dds/xtypes/type_representation/detail/dds-xtypes_typeobjectPubSubTypes.h"+<fastdds/dds/xtypes/type_representation/TypeObject.hpp>+' ./src/cpp/fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.h
sed -i 's+"../../../../../../include/fastdds/dds/core/detail/DDSReturnCodePubSubTypes.h"+<fastdds/dds/core/ReturnCode.hpp>+' ./src/cpp/fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.h

exit $ret_value
