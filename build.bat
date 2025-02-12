@echo off
setlocal

:: Set default values if arguments are not provided
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

set TARGET=%2
if "%TARGET%"=="" set TARGET=all

:: Create and move into the build directory
if not exist build mkdir build
cd build

echo Generating compilation commands...
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=%CONFIG% ..

:: Copy compile_commands.json to the parent directory
copy compile_commands.json ..

:: Use all available logical cores
for /f "tokens=2 delims==" %%n in ('wmic cpu get NumberOfLogicalProcessors /value ^| findstr "="') do set /a nthreads=%%n
if %nthreads% LSS 1 set nthreads=1

echo Starting build using %nthreads% threads...
cmake --build . --config %CONFIG% --target %TARGET% -j %nthreads%

endlocal
