@echo off

rem Generate worker configs
pushd "%~dp0..\..\..\..\..\..\spatial"
spatial worker build build-config
popd

rem Generate schema descriptor
set SCHEMA_COMPILER="%~dp0..\..\Binaries\ThirdParty\Improbable\Programs\schema_compiler.exe"
set PROJECT_SCHEMA="%~dp0..\..\..\..\..\..\spatial\schema"
set GDK_SCHEMA="%~dp0..\..\Extras\schema"
set STANDARD_LIBRARY_SCHEMA="%~dp0..\..\Build\core_sdk\schema\standard_library"
set SCHEMA_DESCRIPTOR_OUT="%~dp0..\..\..\..\..\..\spatial\build\assembly\schema\schema.descriptor"

echo Building schema descriptor

call %SCHEMA_COMPILER% --schema_path=%PROJECT_SCHEMA% --schema_path=%GDK_SCHEMA% --schema_path=%STANDARD_LIBRARY_SCHEMA% --descriptor_set_out=%SCHEMA_DESCRIPTOR_OUT% --load_all_schema_on_schema_path

rem Build Unreal project
pushd "%~dp0..\..\..\..\..\"

set BUILD_EXE_PATH="Plugins\UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Build.exe"

if not exist %BUILD_EXE_PATH% (
	echo Error: Build executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	exit /b 1
)

%BUILD_EXE_PATH% %*

popd

exit /b %ERRORLEVEL%
