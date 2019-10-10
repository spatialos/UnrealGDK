Set-StrictMode -Version Latest

# Expects gdk_home, which is not the GDK location in the engine
param (
    [string] $unreal_path = "$($pwd.drive.root)UnrealEngine"
)

$gdk_in_engine = "$unreal_path\Engine\Plugins\UnrealGDK"

# link the gdk into its expected place in the engine
New-Item -ItemType Junction -Path "$gdk_in_engine" -Target "$gdk_home" -ErrorAction SilentlyContinue

pushd $gdk_in_engine
    if (Get-Variable -Name NO_PAUSE -ErrorAction SilentlyContinue) { # seems like this is set somewhere previously in CI, but just to make sure
        Set-Variable -Name NO_PAUSE -Value 1
    }
    cmd /c Setup.bat
popd
