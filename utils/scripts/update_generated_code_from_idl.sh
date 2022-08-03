#!/bin/bash

idl_files=(
    "./examples/cpp/dds/BasicConfigurationExample/HelloWorld.idl"
    "./examples/cpp/dds/Benchmark/Benchmark.idl"
    "./examples/cpp/dds/Benchmark/Benchmark_big.idl"
    "./examples/cpp/dds/Benchmark/Benchmark_medium.idl"
    "./examples/cpp/dds/Benchmark/Benchmark_small.idl"
    "./examples/cpp/dds/Configurability/sample.idl"
    "./examples/cpp/dds/ContentFilteredTopicExample//HelloWorld.idl"
    "./examples/cpp/dds/CustomListenerExample/Topic.idl"
    "./examples/cpp/dds/DeadlineQoSExample/deadlinepayload.idl"
    "./examples/cpp/dds/DisablePositiveACKs/Topic.idl"
    "./examples/cpp/dds/Filtering/FilteringExample.idl"
    "./examples/cpp/dds/FlowControlExample/FlowControlExample.idl"
    "./examples/cpp/dds/HelloWorldExample/HelloWorld.idl"
    "./examples/cpp/dds/HelloWorldExampleDataSharing/HelloWorld.idl"
    "./examples/cpp/dds/HelloWorldExampleSharedMem/HelloWorld.idl"
    "./examples/cpp/dds/HelloWorldExampleTCP/HelloWorld.idl"
    "./examples/cpp/dds/HistoryKind/sample.idl"
    "./examples/cpp/dds/Keys/sample.idl"
    "./examples/cpp/dds/LateJoiners/sample.idl"
    "./examples/cpp/dds/LifespanQoSExample/Lifespan.idl"
    "./examples/cpp/dds/LivelinessQoS/Topic.idl"
    "./examples/cpp/dds/OwnershipStrengthQoSExample/OwnershipStrength.idl"
    "./examples/cpp/dds/SampleConfig_Controller/sample.idl"
    "./examples/cpp/dds/SampleConfig_Events/sample.idl"
    "./examples/cpp/dds/SampleConfig_Multimedia/sample.idl"
    "./examples/cpp/dds/SecureHelloWorldExample/HelloWorld.idl"
    "./examples/cpp/dds/StaticHelloWorldExample/HelloWorld.idl"
    "./examples/cpp/dds/WriterLoansExample/LoanableHelloWorld.idl"
    "./examples/cpp/dds/ZeroCopyExample/LoanableHelloWorld.idl"
    "./test/blackbox/types/Data64kb.idl"
    "./test/blackbox/types/KeyedData1mb.idl"
    "./test/blackbox/types/HelloWorld.idl"
    "./test/blackbox/types/KeyedHelloWorld.idl"
    "./test/blackbox/types/Data1mb.idl"
    "./test/blackbox/types/FixedSized.idl"
    "./test/blackbox/types/StringTest.idl"
    "./test/unittest/dds/topic/DDSSQLFilter/data_types/ContentFilterTestType.idl"
    "./test/profiling/allocations/AllocTestType.idl"
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
