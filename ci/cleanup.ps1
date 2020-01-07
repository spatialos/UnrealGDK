param (
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine", ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
    [string] $project_path = "TestProject" ## This should ultimately resolve to "C:\b\<number>\TestProject".
)

$project_absolute_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\$project_path"

# Workaround for UNR-2156 and UNR-2076, where spatiald / runtime processes sometimes never close, or where runtimes are orphaned
# Clean up any spatiald and java (i.e. runtime) processes that may not have been shut down
& spatial "service" "stop"
Stop-Process -Name "java" -Force -ErrorAction SilentlyContinue

# Clean up the symlinks
if (Test-Path "$unreal_path") {
    (Get-Item "$unreal_path").Delete()
}

$gdk_in_test_repo = "$project_absolute_path\Game\Plugins\UnrealGDK"
if (Test-Path "$gdk_in_test_repo") {
    (Get-Item "$gdk_in_test_repo").Delete()
}

# Clean up testing project
if (Test-Path $project_absolute_path) {
    Write-Output "Removing existing project"
    Remove-Item $project_absolute_path -Recurse -Force
    if (-Not $?) {
        Throw "Failed to remove existing project at $($project_absolute_path)."
    }
}
