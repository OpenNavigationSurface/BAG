# Unzip, compile, and install all BAG dependencies into the current directory.
#
# Note: Run this as Administrator since some zipfiles contain symlinks.
#

# exec command
function exec
{
  param ( [ScriptBlock] $ScriptBlock )
  & $ScriptBlock 2>&1 | ForEach-Object -Process { "$_" }
  if ($LastExitCode -ne 0) { exit $LastExitCode }
}

# Make sure 7-zip is in the PATH
$env:PATH += ";${env:ProgramFiles}\7-Zip"

# Check that 7-zip is installed
if (-Not (Test-Path -Path "${env:ProgramFiles}\7-Zip\7z.exe")) {
    Write-Error "7-zip is not installed"
    exit 1
}

# Setup VC dev tools
$env:ARCHITECTURE="amd64"

# Find Visual Studio installation path
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = & $vswhere -latest -property installationPath
if ($null -eq $vsPath) {
    Write-Error "Could not find Visual Studio installation"
    exit 1
}

# Setup Visual Studio environment
cmd.exe /c "$vsPath\Common7\Tools\VsDevCmd.bat" -arch=%ARCHITECTURE%
$env:PATH += ";$vsPath\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
$env:VS_VERSION = "Visual Studio 17"

# Project-specific variables
$env:BAG_INSTALL_PATH=Resolve-Path '.\'
$env:ZLIB_ZIP="zlib.zip"
$env:LIBXML2_ZIP="libxml2.zip"
$env:HDF5_ZIP="hdf5.zip"
$env:CATCH2_ZIP="catch2.zip"
$env:BUILD_TYPE_CMAKE="Release"

# zlib
exec { 7z x ..\downloads\$env:ZLIB_ZIP }
cd zlib-1.3
if(-Not (Test-Path -Path build)) { mkdir build }
$env:CMAKE_INSTALL_PREFIX="-DCMAKE_INSTALL_PREFIX=" + $env:BAG_INSTALL_PATH
cmake -B build -G $env:VS_VERSION -S . $env:CMAKE_INSTALL_PREFIX -DCMAKE_BUILD_TYPE=$env:BUILD_TYPE_CMAKE
cmake --build build --config $env:BUILD_TYPE_CMAKE --target install -- /nologo /verbosity:minimal
cd ..
# libxml2
exec { 7z x ..\downloads\$env:LIBXML2_ZIP }
cd libxml2-v2.13.5
if(-Not (Test-Path -Path build)) { mkdir build }
cmake -B build -G $env:VS_VERSION -S . $env:CMAKE_INSTALL_PREFIX -DCMAKE_BUILD_TYPE=$env:BUILD_TYPE_CMAKE `
-DLIBXML2_WITH_ZLIB=ON -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=OFF -DLIBXML2_WITH_PYTHON=OFF
cmake --build build --config $env:BUILD_TYPE_CMAKE --target install -- /nologo /verbosity:minimal
cd ..
# HDF5
exec { 7z x ..\downloads\$env:HDF5_ZIP }
cd hdf5-hdf5_1.14.5
if(-Not (Test-Path -Path build)) { mkdir build }
cmake -B build -G $env:VS_VERSION -S . $env:CMAKE_INSTALL_PREFIX -DCMAKE_BUILD_TYPE=$env:BUILD_TYPE_CMAKE `
-DHDF5_BUILD_CPP_LIB=ON -DHDF5_BUILD_TOOLS:BOOL=OFF `
-DBUILD_TESTING:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=ON `
-DHDF5_BUILD_HL_LIB:BOOL=ON -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON
cmake --build build --config $env:BUILD_TYPE_CMAKE --target install -- /nologo /verbosity:minimal
cd ..
# Catch2
exec { 7z x ..\downloads\$env:CATCH2_ZIP }
cd Catch2-3.4.0
if(-Not (Test-Path -Path build)) { mkdir build }
# Debug: dir
# Debug: pwd
cmake -B build -G $env:VS_VERSION -S . $env:CMAKE_INSTALL_PREFIX -DCMAKE_BUILD_TYPE=$env:BUILD_TYPE_CMAKE `
-DBUILD_TESTING:BOOL=OFF
cmake --build build --config $env:BUILD_TYPE_CMAKE --target install -- /nologo /verbosity:minimal
cd ..

# Cleanup
rmdir -r .\Catch2-3.4.0
rmdir -r .\hdf5-hdf5_1.14.5
rmdir -r .\libxml2-v2.13.5
rmdir -r .\zlib-1.3
