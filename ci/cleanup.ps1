param (
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine" ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
)
$gdk_in_engine = "$unreal_path\Engine\Plugins\UnrealGDK"

# Clean up the symlinks
if (Test-Path "$gdk_in_engine") {
    (Get-Item "$gdk_in_engine").Delete()
}
if (Test-Path "$unreal_path") {
    (Get-Item "$unreal_path").Delete()
}
