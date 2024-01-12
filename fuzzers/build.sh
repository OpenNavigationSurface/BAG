#!/bin/bash -eu

echo "Building BAG for fuzzing..."

cd $SRC/bag

cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build -S . \
  -DCMAKE_PREFIX_PATH='/usr;/usr/local;/usr/local/HDF_Group/HDF5/1.14.3/' \
  -DBAG_BUILD_SHARED_LIBS:BOOL=OFF \
  -DBAG_BUILD_TESTS:BOOL=OFF -DBAG_CODE_COVERAGE:BOOL=OFF \
  -DBAG_BUILD_PYTHON:BOOL=OFF -DBAG_BUILD_EXAMPLES:BOOL=OFF

cmake --build build --config Release --target install

export export LD_LIBRARY_PATH=/usr/lib:/usr/local/lib:/usr/local/HDF_Group/HDF5/1.14.3/lib

SRC_DIR=$SRC/bag

echo "Building bag_read_fuzzer..."
$CXX $CXXFLAGS \
          -I$SRC_DIR/api \
          $(dirname $0)/bag_read_fuzzer.cpp -o $OUT/bag_read_fuzzer \
          $LIB_FUZZING_ENGINE \
          -L/usr/local/lib/static -llibbaglib \
          -Wl,-Bdynamic -ldl -lpthread
