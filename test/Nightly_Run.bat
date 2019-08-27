set TEST_SRC_DIR=%1
set TEST_BIN_DIR=%2
set CONFIG_FILE=%3
set VS_PATH=%4
set CTEST_MODE=Nightly
if NOT [%5]==[] set CTEST_MODE=%5
CALL :dequote VS_PATH

set COMPILER=MSVC
if NOT [%6]==[] set COMPILER=%6

set TEST_DIR=%TEST_SRC_DIR%/test
if NOT [%7]==[] set TEST_DIR=%7

set MAIN_SOLUTION=open_iA.sln
if NOT [%8]==[] set MAIN_SOLUTION=%8

set MODULE_DIRS=%TEST_SRC_DIR%/modules
if NOT [%9]==[] set MODULE_DIRS=%9

:: other variables not changeable via parameters
set BUILD_TYPE=Release
set MSBUILD_OPTS=/t:clean /m

python --version >NUL 2>NUL
IF %ERRORLEVEL% NEQ 0 GOTO PythonNotFound

:: for security reasons (as we call deltree on it) we don't make that configurable
:uniqTmpLoop
set "TEST_CONFIG_PATH=%tmp%\ctest_config_%RANDOM%"
if exist "%TEST_CONFIG_PATH%" goto :uniqTmpLoop

::TO PAUSE SYSTEM
:: ping -n 10 127.0.0.1 > null
cd %TEST_SRC_DIR%
FOR /F "tokens=1 delims=" %%A in ('git symbolic-ref --short HEAD') do SET GIT_BRANCH=%%A
rem echo %GIT_BRANCH%

:: git pull origin master

:: Set up Visual Studio Environment for cleaning build
:: amd64 is the target architecture (see http://msdn.microsoft.com/en-us/library/x4d2c09s%28v=vs.80%29.aspx)
echo %VS_PATH% | findstr /c:"2017" >nul
if %ERRORLEVEL% EQU 0 goto VS2017
call "%VS_PATH%\VC\vcvarsall.bat" amd64
goto VCVARSEnd
:VS2017
 :: since VS 2017, vcvarsall.bat is in a different subdirectory...
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" amd64
:VCVARSEnd

@echo on

:: just in case it doesn't exist yet, create output directory:
md %TEST_BIN_DIR%
cd %TEST_BIN_DIR%
if %ERRORLEVEL% GEQ 1 goto BinDirError

:: Set up basic build environment
cmake -C "%CONFIG_FILE%" %TEST_SRC_DIR% 2>&1

:: Create test configurations:
md %TEST_CONFIG_PATH%
python %TEST_DIR%\CreateTestConfigurations.py %TEST_SRC_DIR% %GIT_BRANCH% %TEST_CONFIG_PATH% %MODULE_DIRS% %COMPILER%

rem Run with all flags enabled:
cmake -C %TEST_CONFIG_PATH%\all_flags.cmake %TEST_SRC_DIR% 2>&1
MSBuild "%TEST_BIN_DIR%\%MAIN_SOLUTION%" %MSBUILD_OPTS%
:: del %TEST_Bin_DIR%\core\moc_*
:: del %TEST_Bin_DIR%\modules\moc_*
ctest -D %CTEST_MODE% -C %BUILD_TYPE%

rem Run with no flags enabled:
cmake -C %TEST_CONFIG_PATH%\no_flags.cmake %TEST_SRC_DIR% 2>&1
MSBuild "%TEST_BIN_DIR%\%MAIN_SOLUTION%" %MSBUILD_OPTS%
:: del %TEST_Bin_DIR%\core\moc_*
:: del %TEST_Bin_DIR%\modules\moc_*
ctest -D Experimental -C %BUILD_TYPE%

del %TEST_BIN_DIR%\Testing\Temporary\*.mhd %TEST_BIN_DIR%\Testing\Temporary\*.raw

:: iterate over module tests, run build&tests for each:
FOR %%m IN (%TEST_CONFIG_PATH%\Module_*) DO @(
	@echo(
	@echo ================================================================================
	@echo %%~nm
	@echo ================================================================================
	cmake -C %TEST_CONFIG_PATH%\no_flags.cmake %TEST_SRC_DIR% 2>&1
	cmake -C %%m %TEST_SRC_DIR% 2>&1
	MSBuild "%TEST_BIN_DIR%\%MAIN_SOLUTION%" %MSBUILD_OPTS%
	:: del %TEST_Bin_DIR%\core\moc_*
	:: del %TEST_Bin_DIR%\modules\moc_*
	ctest -D Experimental -C %BUILD_TYPE%
	del %TEST_BIN_DIR%\Testing\Temporary\*.mhd %TEST_BIN_DIR%\Testing\Temporary\*.raw
)

:: CLEANUP:
:: move out of %TEST_BIN_DIR%
cd ..

:: remove test configurations:
rd /s /q %TEST_CONFIG_PATH%

:: wait for 10 seconds before deleting:
ping 127.0.0.1 -n 11 -w > nul
:: remove binary directory to start from scratch next time:
rd /s /q %TEST_BIN_DIR%

goto end

:DeQuote
for /f "delims=" %%A in ('echo %%%1%%') do set %1=%%~A
Goto :eof

:PythonNotFound
echo Python wasn't found in the PATH! Please install/set PATH appropriately!
goto end

:BinDirError
echo Could not create or enter binary directory, exiting!
goto end

:end
