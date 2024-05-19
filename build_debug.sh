# /bin/bash

cmake -H. -Bbuild -DCOMPILE_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build ./build
export LD_LIBRARY_PATH=/FastDDS/build/src/cpp:$LD_LIBRARY_PATH
