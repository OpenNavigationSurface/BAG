#!/bin/bash -eu

echo "Building BAG for fuzzing..."

SRC_DIR=$SRC/bag
cd $SRC_DIR

cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build -S . \
  -DCMAKE_INSTALL_PREFIX:PATH=/opt \
  -DCMAKE_PREFIX_PATH='/opt;/opt/local;/opt/local/HDF_Group/HDF5/1.14.3/' \
  -DBAG_BUILD_SHARED_LIBS:BOOL=OFF \
  -DBAG_BUILD_TESTS:BOOL=OFF -DBAG_CODE_COVERAGE:BOOL=OFF \
  -DBAG_BUILD_PYTHON:BOOL=OFF -DBAG_BUILD_EXAMPLES:BOOL=OFF

cmake --build build --config Release --target install

echo "Building bag_read_fuzzer..."
$CXX $CXXFLAGS \
  -I$SRC_DIR/api \
  fuzzers/bag_read_fuzzer.cpp -o $OUT/bag_read_fuzzer \
  $LIB_FUZZING_ENGINE \
  -L/opt/lib/static -lbaglib \
  -L/opt/lib -lhdf5_cpp \
  -L/opt/lib -lhdf5 \
  -L/opt/lib -lxml2

echo "Building bag_extended_fuzzer..."
$CXX $CXXFLAGS \
  -I$SRC_DIR/api \
  fuzzers/bag_extended_fuzzer.cpp -o $OUT/bag_extended_fuzzer \
  $LIB_FUZZING_ENGINE \
  -L/opt/lib/static -lbaglib \
  -L/opt/lib -lhdf5_cpp \
  -L/opt/lib -lhdf5 \
  -L/opt/lib -lxml2

echo "Building seed corpus..."
zip -j $OUT/bag_extended_fuzzer_seed_corpus.zip $SRC_DIR/examples/sample-data/*.bag
