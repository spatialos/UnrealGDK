# Expects gdk_home, which is not the GDK location in the engine
param (
    [string] $gdk_path,
    [string] $msbuild_path
)

pushd $gdk_path
    if (-Not (Test-Path env:NO_PAUSE)) { # seems like this is set somewhere previously in CI, but just to make sure
        $env:NO_PAUSE = 1
    }
    $env:MSBUILD_EXE = "`"$msbuild_path`""
    cmd /c Setup.bat
popd
