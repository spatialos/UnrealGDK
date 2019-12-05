param (
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine" ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
    [string] $project_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\TestProject" ## This should ultimately resolve to "C:\b\<number>\TestProject".
)

# Clean up the symlinks
if (Test-Path "$unreal_path") {
    (Get-Item "$unreal_path").Delete()
}
$gdk_in_test_repo = "$project_path\Game\Plugins\UnrealGDK"
if (Test-Path "$gdk_in_test_repo") {
    (Get-Item "$gdk_in_test_repo").Delete()
}

# Workaround for UNR-2156 and UNR-2076, where spatiald / runtime processes sometimes never close, or where runtimes are orphaned
# Clean up any spatiald and java (i.e. runtime) processes that may not have been shut down
& spatial "service","stop" -Wait -ErrorAction Stop -NoNewWindow
Stop-Process -Name "java" -Force -ErrorAction SilentlyContinue

# Clean up testing project
if (Test-Path $project_path) {
    Write-Host "Removing existing project"
    Remove-Item $project_path -Recurse -Force
    if (-Not $?) {
        Throw "Failed to remove existing project at $($project_path)."
    }
}
