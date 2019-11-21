param(
    [string] $build_output_dir,
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine", ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
    [string] $test_repo_branch,
    [string] $test_repo_url,
    [string] $test_repo_uproject_path,
    [string] $test_repo_path,
    [string] $msbuild_exe,
    [string] $gdk_home
)

# Copy the built files back into the SpatialGDK folder, to have a complete plugin
# The trailing \ on the destination path is important!
Copy-Item -Path "$build_output_dir\*" -Destination "$gdk_home\SpatialGDK\" -Recurse -Container -ErrorAction SilentlyContinue

# Clean up testing project (symlinks could be invalid during initial cleanup - leaving the project as a result)
if (Test-Path $test_repo_path) {
    Write-Log "Removing existing project"
    Remove-Item $test_repo_path -Recurse -Force
    if (-Not $?) {
        Throw "Failed to remove existing project at $($test_repo_path)."
    }
}

# Clone and build the testing project
Write-Log "Downloading the testing project from $($test_repo_url)"
Git clone -b "$test_repo_branch" "$test_repo_url" "$test_repo_path" --depth 1
if (-Not $?) {
    Throw "Failed to clone testing project from $($test_repo_url)."
}

# The Plugin does not get recognised as an Engine plugin, because we are using a pre-built version of the engine
# copying the plugin into the project's folder bypasses the issue
New-Item -ItemType Junction -Name "UnrealGDK" -Path "$test_repo_path\Game\Plugins" -Target "$gdk_home"

Write-Log "Generating project files"
Start-Process "$unreal_path\Engine\Binaries\DotNET\UnrealBuildTool.exe" "-projectfiles","-project=`"$test_repo_uproject_path`"","-game","-engine","-progress" -Wait -ErrorAction Stop -NoNewWindow
if (-Not $?) {
    throw "Failed to generate files for the testing project."
}

Write-Log "Building the testing project"
Start-Process "$msbuild_exe" -Wait -ErrorAction Stop -NoNewWindow -ArgumentList @(`
    "/nologo", `
    "$($test_repo_uproject_path.Replace(".uproject", ".sln"))", `
    "/p:Configuration=`"$env:BUILD_STATE $env:BUILD_TARGET`";Platform=`"$env:BUILD_PLATFORM`""
)
if (-Not $?) {
    throw "Failed to build testing project."
}
