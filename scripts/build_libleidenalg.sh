LIBLEIDENALG_VERSION=0.11.0

ROOT_DIR=`pwd`
echo "Using root dir ${ROOT_DIR}"

# Create source directory
if [ ! -d "${ROOT_DIR}/build-deps/src" ]; then
  echo ""
  echo "Make directory ${ROOT_DIR}/build-deps/src"
  mkdir -p ${ROOT_DIR}/build-deps/src
fi

cd ${ROOT_DIR}/build-deps/src
if [ ! -d "libleidenalg" ]; then
  echo ""
  echo "Cloning libleidenalg into ${ROOT_DIR}/build-deps/src/libleidenalg"
  # Clone repository if it does not exist yet
  git clone --branch ${LIBLEIDENALG_VERSION} https://github.com/vtraag/libleidenalg.git --single-branch
fi

# Make sure the git repository points to the correct version
echo ""
echo "Checking out ${LIBLEIDENALG_VERSION} in ${ROOT_DIR}/build-deps/src/libleidenalg"
cd ${ROOT_DIR}/build-deps/src/libleidenalg
git fetch origin tag ${LIBLEIDENALG_VERSION} --no-tags
git checkout ${LIBLEIDENALG_VERSION}

# Make build directory
if [ ! -d "${ROOT_DIR}/build-deps/build/libleidenalg" ]; then
  echo ""
  echo "Make directory ${ROOT_DIR}/build-deps/build/libleidenalg"
  mkdir -p ${ROOT_DIR}/build-deps/build/libleidenalg
fi

# Configure, build and install
cd ${ROOT_DIR}/build-deps/build/libleidenalg

echo ""
echo "Configure libleidenalg build"
cmake ${ROOT_DIR}/build-deps/src/libleidenalg \
    -DCMAKE_INSTALL_PREFIX=${ROOT_DIR}/build-deps/install/ \
    -DBUILD_SHARED_LIBS=ON \
    -Digraph_ROOT=${ROOT_DIR}/build-deps/install/lib/cmake/igraph/ \
    ${EXTRA_CMAKE_ARGS}

echo ""
echo "Build libleidenalg"
cmake --build .

echo ""
echo "Install libleidenalg to ${ROOT_DIR}/build-deps/install/"
cmake --build . --target install
