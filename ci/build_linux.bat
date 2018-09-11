@echo off

pushd "%~dp0../SpatialGDK"

call Setup.bat

popd

pushd "%~dp0../SpatialGDK"

call "%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="%~dp0../SpatialGDK/SpatialGDK.uplugin" -TargetPlatforms=Linux -Package=Temp

popd

pause
