@echo off

pushd "%~dp0../SpatialGDK"

call Setup.bat

call "%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="%~dp0../SpatialGDK/SpatialGDK.uplugin" -TargetPlatforms=Linux -Package="%~dp0../SpatialGDK/Intermediate/BuildPackage/Linux"

popd

pause
