rem **** Warning - Experimental functionality ****
rem We do not support this functionality currently: Do not use it unless you are Improbable staff.
rem **** 

@echo off

set NO_PAUSE=1
set NO_SET_LOCAL=1

pushd "%~dp0"

call Setup.bat %*

call :MarkStartOfBlock "%~0"

call :MarkStartOfBlock "Create folders"
	md "%CORE_SDK_DIR%\trace_lib"    >nul 2>nul
call :MarkEndOfBlock "Create folders"

call :MarkStartOfBlock "Retrieve dependencies"	
	spatial package retrieve internal        trace-dynamic-x86_64-vc141_md-win32        %PINNED_CORE_SDK_VERSION% 	"%CORE_SDK_DIR%\trace_lib\trace-win32.zip"
	spatial package retrieve internal        trace-dynamic-x86_64-clang1000-linux       %PINNED_CORE_SDK_VERSION% 	"%CORE_SDK_DIR%\trace_lib\trace-linux.zip"
call :MarkEndOfBlock "Retrieve dependencies"

REM There is a race condition between retrieve and unzip, add version call to stall briefly
call spatial version 

call :MarkStartOfBlock "Unpack dependencies"
    powershell -Command "Expand-Archive -Path \"%CORE_SDK_DIR%\trace_lib\trace-win32.zip\"	-DestinationPath \"%BINARIES_DIR%\Win64\" -Force;"^
						"Expand-Archive -Path \"%CORE_SDK_DIR%\trace_lib\trace-linux.zip\"	-DestinationPath \"%BINARIES_DIR%\Linux\" -Force;"
	xcopy /s /i /q "%BINARIES_DIR%\Win64\include\improbable" "%WORKER_SDK_DIR%\improbable\legacy"

	set LEGACY_FOLDER=%WORKER_SDK_DIR%\improbable\legacy\
	set TRACE_HEADER="%LEGACY_FOLDER%trace.h"
	powershell -Command "(Get-Content '%TRACE_HEADER%').replace('#include <improbable/c_trace.h>', '#include <improbable/legacy/c_trace.h>') | Set-Content -Force '%TRACE_HEADER%'"
	REM These modifications are temporary fixes until worker packages post 15.0.0-preview-4 is released
	powershell -Command "(Get-Content '%TRACE_HEADER%').replace('void SetIntervalMillis', 'inline void SetIntervalMillis') | Set-Content -Force '%TRACE_HEADER%'"
	powershell -Command "(Get-Content '%TRACE_HEADER%').replace('void SetBatchSize', 'inline void SetBatchSize') | Set-Content -Force '%TRACE_HEADER%'"
	
call :MarkEndOfBlock "Unpack dependencies"

call :MarkEndOfBlock "%~0"

popd

exit /b %ERRORLEVEL%

:MarkStartOfBlock
echo Starting: %~1
exit /b 0

:MarkEndOfBlock
echo Finished: %~1
exit /b 0
