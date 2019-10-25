param (
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine" ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
)
$gdk_in_engine = "$unreal_path\Engine\Plugins\UnrealGDK"

# Clean up the symlinks
if (Test-Path "$gdk_in_engine") {
    (Get-Item "$gdk_in_engine").Delete()
}
if (Test-Path "$unreal_path\Samples\UnrealGDKCITestProject\Plugins\UnrealGDK") {
    (Get-Item "$unreal_path\Samples\UnrealGDKCITestProject\Plugins\UnrealGDK").Delete() # TODO needs to stay in sync with setup-tests
}
if (Test-Path "$unreal_path") {
    (Get-Item "$unreal_path").Delete()
}

# Clean up testing project
$testing_project_path = "$unreal_path\Samples\UnrealGDKCITestProject"
if (Test-Path $testing_project_path) {

    # Stop potential running spatial service before removing the project
    Start-Process spatial "service","stop" -Wait -ErrorAction Stop -NoNewWindow

    Write-Log "Removing existing project."
    Remove-Item $testing_project_path -Recurse -Force
    if (-Not $?) {
        Throw "Failed to remove existing project at $($testing_project_path)."
    }
}
