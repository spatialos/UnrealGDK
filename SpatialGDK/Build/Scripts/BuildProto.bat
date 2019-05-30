@echo off

pushd "%~dp0..\..\..\"

rem We expect a path to the 'schema' directory in the 'spatial' folder and to where the schema.descriptor should be saved.
rem Example: 'spatial\schema'
set SchemaDir=%1

set SchemaDir2=%2

rem Example: 'spatial\build\descriptor\output'
set SchemaDescriptorDir=%3

set SCHEMA_COMPILER_EXE_PATH="SpatialGDK\Binaries\ThirdParty\Improbable\Programs\schema_compiler.exe"

if not exist %SCHEMA_COMPILER_EXE_PATH% (
	echo Error: schema_compiler executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	pause
	exit /b 1
)

%SCHEMA_COMPILER_EXE_PATH% --schema_path=%SchemaDir% --schema_path=%SchemaDir2% --descriptor_set_out=%SchemaDescriptorDir% --load_all_schema_on_schema_path

popd

if ERRORLEVEL 1 (
	pause
) else (
	echo Successfully generated proto descriptor!
)
exit /b %ERRORLEVEL%
