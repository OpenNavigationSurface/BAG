echo off
echo ***************************************************************************
echo Build.bat can be used in 5 ways                                           *
echo build          - Builds the code (using existing projects/build status)   *
echo build clean    - Deletes any library/apps/object files                    *
echo build project  - Rebuilds the DSP and Makefiles from the project.         *
echo build rebuild  - Does a clean, then a build (but doesn't rebuild projects *
echo build world    - Does a clean, rebuilts the projects, builds the code     *
echo ***************************************************************************                                           

if "%1" =="clean"   goto build_clean
if "%1" =="rebuild" goto build_rebuild
if "%1" =="world"   goto build_world
if "%1" =="project"   goto build_project

echo Building verifycert application
nmake -f Makefile_win32
goto end1

:build_clean
echo Cleaning verifycert application
nmake -f Makefile_win32 clean
goto end1

:build_project
echo Building verifycert application project
qmake -t app -o Makefile_win32 verifycert.pro
qmake verifycert.pro
goto end1

:build_rebuild
echo Rebuilding verifycert application
call build clean
call build
goto end1

:build_world
echo Building the world for the verifycert application
call build clean
call build project
call build
goto end1

:end1
