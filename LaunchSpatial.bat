@echo off

pushd %~dp0

call SpatialGDK\Build\Scripts\BuildProto.bat

spatial local launch ^
	--main_config=SpatialConfig\SpatialOS.json ^
	--world=SpatialConfig\World.json ^
	--launch_config=SpatialConfig\Runtime.json ^
	--log_directory=SpatialArtifacts\Logs ^
	--optimize_for_runtime_v2

popd

pause
