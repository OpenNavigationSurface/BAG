# Cleanup
if(test-path build) {
    Remove-Item -Recurse -Force build
}
if(test-path wheel) {
    Remove-Item -Recurse -Force wheel
}

# Find Visual Studio installation path
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = & $vswhere -latest -property installationPath
if ($null -eq $vsPath) {
    Write-Error "Could not find Visual Studio installation"
    exit 1
}

$env:ARCHITECTURE="amd64"
. "$vsPath\Common7\Tools\Launch-VsDevShell.ps1" -Arch $env:ARCHITECTURE

# Make sure the current directory is the content root of the repo after setting up VsDevShell
$content_root=Resolve-Path "$PSScriptRoot\.."
pushd $content_root

$env:CC="cl.exe"
$env:CXX="cl.exe"
cmake -G "Visual Studio 17 2022" -B build -S .  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=docs\win-build\baglibs\install `
  -DCMAKE_INSTALL_PREFIX=docs\win-build\baglibs\install -DBUILD_SHARED_LIBS=ON `
  -DBAG_BUILD_TESTS:BOOL=ON -DBAG_CI=ON -DCMAKE_OBJECT_PATH_MAX=1024
cmake --build build --config Release --target install
$env:PATH=$env:PATH + ";${content_root}\build\api\Release;${content_root}\docs\win-build\baglibs\install\bin"
$env:BAG_SAMPLES_PATH="${content_root}\examples\sample-data"
build\tests\Release\bag_tests.exe
python -m venv win-venv
& .\win-venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
$env:CMAKE_PREFIX_PATH="$PWD\docs\win-build\baglibs\install"
$env:SWIG_EXECUTABLE="$PWD\docs\win-build\baglibs\install\swigwin-4.3.0\swig.exe"
python -m pip wheel -w .\wheel\ .\build\api\swig\python
$whl_path=Resolve-Path ".\wheel\bagPy-*.whl"
python -m pip install $whl_path
$env:BAG_BIN = "${content_root}\docs\win-build\baglibs\install\bin"
python -m pytest .\python\test_compat_bag15.py .\python\test_compounddatatype.py .\python\test_dataset.py .\python\test_descriptor.py `
            .\python\test_interleavedlegacylayer.py .\python\test_interleavedlegacylayerdescriptor.py `
            .\python\test_metadata.py .\python\test_record.py .\python\test_simplelayer.py `
            .\python\test_simplelayerdescriptor.py .\python\test_surfacecorrections.py `
            .\python\test_surfacecorrectionsdescriptor.py .\python\test_trackinglist.py .\python\test_valuetable.py `
            .\python\test_vrmetadata.py .\python\test_vrmetadatadescriptor.py .\python\test_vrnode.py `
            .\python\test_vrnodedescriptor.py .\python\test_vrrefinements.py .\python\test_vrrefinementsdescriptor.py `
            .\python\test_vrtrackinglist.py
