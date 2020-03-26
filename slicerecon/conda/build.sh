#!/bin/sh

cd slicerecon

export PKG_CONFIG_PATH=$CONDA_PREFIX/lib/pkgconfig/:$PKG_CONFIG_PATH

declare -a CMAKE_PLATFORM_FLAGS
if [[ ${HOST} =~ .*linux.* ]]; then
    CMAKE_PLATFORM_FLAGS+=(-DCMAKE_TOOLCHAIN_FILE="${RECIPE_DIR}/cross-linux.cmake")
fi

cd python
$PYTHON setup.py install --single-version-externally-managed --record=record.txt
cd ..

mkdir build && cd build

cmake ..                                        \
      -DCMAKE_INSTALL_PREFIX=$PREFIX            \
      -DCMAKE_INSTALL_LIBDIR=$PREFIX/lib        \
      -DCMAKE_BUILD_TYPE=Release                \
      ${CMAKE_PLATFORM_FLAGS[@]}

make -j $CPU_COUNT VERBOSE=1
make install

