@echo off

pushd "%~dp0..\..\..\"

set SCHEMA_COMPILER_EXE_PATH="SpatialGDK\Binaries\ThirdParty\Improbable\Programs\schema_compiler.exe"

if not exist %SCHEMA_COMPILER_EXE_PATH% (
	echo Error: schema_compiler executable not found! Please run Setup.bat in your UnrealGDK root to generate it.
	pause
	exit /b 1
)

%SCHEMA_COMPILER_EXE_PATH% --schema_path="SpatialArtifacts\Schema" --schema_path="SpatialConfig\Schema" --descriptor_set_out="SpatialArtifacts\Schema.descriptor" --load_all_schema_on_schema_path

popd

if ERRORLEVEL 1 (
	pause
) else (
	echo Successfully generated proto descriptor!
)
exit /b %ERRORLEVEL%
