param(
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine", ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
    [string] $test_repo_map,
    [string] $test_repo_uproject_path,
    [string] $test_repo_path
)

# Generate schema and snapshots
Echo "Generating snapshot and schema for testing project"
$commandlet_process = Start-Process "$unreal_path\Engine\Binaries\Win64\UE4Editor.exe" -Wait -PassThru -NoNewWindow -ArgumentList @(`
    "$test_repo_uproject_path", `
    "-run=GenerateSchemaAndSnapshots", `
    "-MapPaths=`"$test_repo_map`"", `
    "-NoShaderCompile"
)
if (-Not $?) {
    Write-Log $commandlet_process.
    throw "Failed to generate schema and snapshots."
}

# Create the default snapshot
Copy-Item -Force `
    -Path "$test_repo_path\spatial\snapshots\$test_repo_map.snapshot" `
    -Destination "$test_repo_path\spatial\snapshots\default.snapshot"

# Create the TestResults directory if it does not exist, for storing results
New-Item -Path "$PSScriptRoot" -Name "TestResults" -ItemType "directory" -ErrorAction SilentlyContinue
