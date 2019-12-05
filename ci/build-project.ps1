param(
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine", ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
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
Echo "Downloading the testing project from $($test_repo_url)"
git clone -b "$test_repo_branch" "$test_repo_url" "$test_repo_path" --depth 1
if (-Not $?) {
    Throw "Failed to clone testing project from $($test_repo_url)."
}

# The Plugin does not get recognised as an Engine plugin, because we are using a pre-built version of the engine
# copying the plugin into the project's folder bypasses the issue
New-Item -ItemType Junction -Name "UnrealGDK" -Path "$test_repo_path\Game\Plugins" -Target "$gdk_home"

# Disable tutorials, otherwise the closing of the window will crash the editor due to some graphic context reason
Add-Content -Path "$unreal_path\Engine\Config\BaseEditorSettings.ini" -Value "`r`n[/Script/IntroTutorials.TutorialStateSettings]`r`nTutorialsProgress=(Tutorial=/Engine/Tutorial/Basics/LevelEditorAttract.LevelEditorAttract_C,CurrentStage=0,bUserDismissed=True)`r`n"

Echo "Generating project files"
$proc = Start-Process "$unreal_path\Engine\Binaries\DotNET\UnrealBuildTool.exe" -Wait -ErrorAction Stop -NoNewWindow -PassThru -ArgumentList @(`
    "-projectfiles", `
    "-project=`"$test_repo_uproject_path`"", `
    "-game", `
    "-engine", `
    "-progress"
)
if ($proc.ExitCode -ne 0) {
    throw "Failed to generate files for the testing project."
}

Echo "Building project"
$build_configuration = $build_state + $(If ("$build_target" -eq "") {""} Else {" $build_target"})
$proc = Start-Process "$msbuild_exe" -Wait -ErrorAction Stop -NoNewWindow -PassThru -ArgumentList @(`
    "/nologo", `
    "$($test_repo_uproject_path.Replace(".uproject", ".sln"))", `
    "/p:Configuration=`"$build_configuration`";Platform=`"$build_platform`""
)
if ($proc.ExitCode -ne 0) {
    throw "Failed to build testing project."
}
