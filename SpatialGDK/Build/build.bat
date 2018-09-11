pushd "%~dp0../Extras"

call BuildGDK.bat

popd

pushd "%~dp0"

call "%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -plugin="%~dp0../SpatialGDK.uplugin" -package=Temp

popd

pause
