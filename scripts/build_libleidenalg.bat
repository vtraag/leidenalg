@echo off

set LIBLEIDENALG_VERSION="0.11.0"

set ROOT_DIR=%cd%
echo Using root dir %ROOT_DIR%

if not exist "%ROOT_DIR%\build-deps\src\" (
  md %ROOT_DIR%\build-deps\src
)

cd "%ROOT_DIR%\build-deps\src"
if not exist "libleidenalg\" (
  echo.
  echo Cloning libleidenalg into %ROOT_DIR%\build-deps\src\libleidenalg
  REM Clone repository if it does not exist yet
  git clone --depth 1 --branch %libleidenalg_VERSION% https://github.com/vtraag/libleidenalg.git
)

REM Make sure the git repository points to the correct version
echo.
echo Checking out %libleidenalg_VERSION} in ${ROOT_DIR%\build-deps\src\libleidenalg
cd "%ROOT_DIR%\build-deps\src\libleidenalg"
git fetch origin tag ${LIBLEIDENALG_VERSION} --no-tags
git checkout ${LIBLEIDENALG_VERSION}

REM Make build directory
if not exist "%ROOT_DIR%\build-deps\build\libleidenalg\" (
  echo.
  echo Make directory %ROOT_DIR%\build-deps\build\libleidenalg
  md %ROOT_DIR%\build-deps\build\libleidenalg
)

REM Configure, build and install
cd "%ROOT_DIR%\build-deps\build\libleidenalg"

echo.
echo Configure libleidenalg build
cmake %ROOT_DIR%\build-deps\src\libleidenalg ^
    -DCMAKE_INSTALL_PREFIX=%ROOT_DIR%\build-deps\install\ ^
    -DBUILD_SHARED_LIBS=ON ^
    -Digraph_ROOT=%ROOT_DIR%\build-deps\install\lib\cmake\igraph\ ^
    %EXTRA_CMAKE_ARGS%

echo.
echo Build libleidenalg
cmake --build . --config Release

echo.
echo Install libleidenalg to %ROOT_DIR%\build-deps\install\
cmake --build . --target install --config Release

cd "%ROOT_DIR%"