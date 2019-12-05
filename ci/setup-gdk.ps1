# Expects gdk_home, which is not the GDK location in the engine
# This script is used directly as part of the UnrealGDKExampleProject CI, so providing default values is strictly necessary
param (
    [string] $gdk_path = "$gdk_home",
    [string] $msbuild_path = "$((Get-Item 'Env:programfiles(x86)').Value)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild.exe" ## Location of MSBuild.exe on the build agent, as it only has the build tools, not the full visual studio
)

pushd $gdk_path
    if (-Not (Test-Path env:NO_PAUSE)) { # seems like this is set somewhere previously in CI, but just to make sure
        $env:NO_PAUSE = 1
    }
    $env:MSBUILD_EXE = "`"$msbuild_path`""
    cmd /c Setup.bat
popd
