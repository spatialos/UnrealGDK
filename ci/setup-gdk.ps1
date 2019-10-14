# Expects gdk_home, which is not the GDK location in the engine
param (
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine",
    [string] $msbuild_path = "$((Get-Item 'Env:programfiles(x86)').Value)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild.exe"
)

$gdk_in_engine = "$unreal_path\Engine\Plugins\UnrealGDK"

# link the gdk into its expected place in the engine
New-Item -ItemType Junction -Path "$gdk_in_engine" -Target "$gdk_home" -ErrorAction SilentlyContinue

pushd $gdk_in_engine
    if (-Not (Test-Path env:NO_PAUSE)) { # seems like this is set somewhere previously in CI, but just to make sure
        $env:NO_PAUSE = 1
    }
    $env:MSBUILD_EXE = "`"$msbuild_path`""
    cmd /c Setup.bat
popd
