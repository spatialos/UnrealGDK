param(
    [string] $unreal_path,
    [string] $test_repo_branch,
    [string] $test_repo_url,
    [string] $test_repo_uproject_path,
    [string] $test_repo_path,
    [string] $msbuild_exe,
    [string] $gdk_home,
    [string] $build_platform,
    [string] $build_state,
    [string] $build_target
)

# Clone the testing project
Write-Output "Downloading the testing project from $($test_repo_url)"
git clone -b "$test_repo_branch" "$test_repo_url" "$test_repo_path" --depth 1
if (-Not $?) {
    Throw "Failed to clone testing project from $test_repo_url."
}

# The Plugin does not get recognised as an Engine plugin, because we are using a pre-built version of the engine
# copying the plugin into the project's folder bypasses the issue
New-Item -ItemType Junction -Name "UnrealGDK" -Path "$test_repo_path\Game\Plugins" -Target "$gdk_home"

Write-Output "Generating project files"
& "$unreal_path\Engine\Binaries\DotNET\UnrealBuildTool.exe" `
    "-projectfiles" `
    "-project=`"$test_repo_uproject_path`"" `
    "-game" `
    "-engine" `
    "-progress"
if ($lastExitCode -ne 0) {
    throw "Failed to generate files for the testing project."
}

Write-Output "Building project"
$build_configuration = $build_state + $(If ("$build_target" -eq "") { "" } Else { " $build_target" })
& "$msbuild_exe" `
    "/nologo" `
    "$($test_repo_uproject_path.Replace(".uproject", ".sln"))" `
    "/p:Configuration=`"$build_configuration`";Platform=`"$build_platform`""
if ($lastExitCode -ne 0) {
    throw "Failed to build testing project."
}
