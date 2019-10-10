Set-StrictMode -Version Latest

param (
    [string] $unreal_path = "$($pwd.drive.root)UnrealEngine"
)
$gdk_in_engine = "$unreal_path\Engine\Plugins\UnrealGDK"

    # clean up the symlink
if (Test-Path "$gdk_in_engine") {
    (Get-Item "$gdk_in_engine").Delete()
}
if (Test-Path "$unreal_path\Samples\StarterContent\Plugins\UnrealGDK") {
    (Get-Item "$unreal_path\Samples\StarterContent\Plugins\UnrealGDK").Delete() # TODO needs to stay in sync with setup-tests
}
