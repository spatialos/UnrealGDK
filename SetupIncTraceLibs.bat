rem **** Warning - Experimental functionality ****
rem We do not support this functionality currently: Do not use it unless you are Improbable staff.
rem **** 

@echo off

set NO_PAUSE=1
set NO_SET_LOCAL=1
call Setup.bat

pushd "%~dp0"

call :MarkStartOfBlock "%~0"

call :MarkStartOfBlock "Create folders"
	md "%CORE_SDK_DIR%\trace_lib"    >nul 2>nul
call :MarkEndOfBlock "Create folders"

call :MarkStartOfBlock "Retrieve dependencies"	
	spatial package retrieve internal        trace-dynamic-x86_64-vc140_md-win32        14.3.0-b2647-85717ee-WORKER-SNAPSHOT "%CORE_SDK_DIR%\trace_lib\trace-win32.zip"
	spatial package retrieve internal        trace-dynamic-x86_64-gcc510-linux          14.3.0-b2647-85717ee-WORKER-SNAPSHOT "%CORE_SDK_DIR%\trace_lib\trace-linux.zip"
call :MarkEndOfBlock "Retrieve dependencies"

call :MarkStartOfBlock "Unpack dependencies"
    powershell -Command "Expand-Archive -Path \"%CORE_SDK_DIR%\trace_lib\trace-win32.zip\"	-DestinationPath \"%BINARIES_DIR%\Win64\" -Force;"^
						"Expand-Archive -Path \"%CORE_SDK_DIR%\trace_lib\trace-linux.zip\"	-DestinationPath \"%BINARIES_DIR%\Linux\" -Force;"
	xcopy /s /i /q "%BINARIES_DIR%\Win64\improbable" "%WORKER_SDK_DIR%\improbable"
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
