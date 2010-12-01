echo off
echo ***************************************************************************
echo Build.bat can be used in 5 ways                                           *
echo build          - Builds the code (using existing projects/build status)   *
echo build clean    - Deletes any library/apps/object files                    *
echo build project  - Rebuilds the DSP and Makefiles from the project.         *
echo build rebuild  - Does a clean, then a build (but doesn't rebuild projects *
echo build world    - Does a clean, rebuilts the projects, builds the code     *
echo ***************************************************************************                                           

if "%1" =="project"   goto project
if "%1" =="build"   goto build
if "%1" =="clean"   goto clean
if "%1" =="rebuild" goto rebuild
if "%1" =="world"   goto world

goto end1

:project
echo Master Batch File to create the project for external libraries (project)

REM No setup for the FlexLM libraries
cd hdf5-1.8.5-patch1
call build project
cd ..
cd zlib-1.2.5
call build project
cd ..
cd szip
call build project
cd ..
cd beecrypt
call build project
cd ..
goto endl

:build
echo Master Batch File to build all external libraries (build)
cd hdf5-1.8.5-patch1
call build build
cd ..
cd zlib-1.2.5
call build build
cd ..
cd szip
call build build
cd ..
cd beecrypt
call build build
cd ..
cd xerces
call build build
cd ..
goto endl

:clean
echo Master Batch File to clean all external libraries (clean)
cd hdf5-1.8.5-patch1
call build clean
cd ..
cd zlib-1.2.5
call build clean
cd ..
cd szip
call build clean
cd ..
cd beecrypt
call build clean
cd ..
cd xerces
call build clean
cd ..
goto endl

:rebuild
echo Master Batch File to re-build all external libraries (rebuild)
cd hdf5-1.8.5-patch1
call build rebuild
cd ..
cd zlib-1.2.5
call build rebuild
cd ..
cd szip
call build rebuild
cd ..
cd beecrypt
call build rebuild
cd ..
cd xerces
call build rebuild
cd ..
goto endl

:world
echo Master Batch File to rebuild the project files and all external libraries (world)
cd hdf5-1.8.5-patch1
call build world
cd ..
cd zlib-1.2.5
call build world
cd ..
cd szip
call build world
cd ..
cd beecrypt
call build world
cd ..
cd xerces
call build world
cd ..
goto endl

:endl

