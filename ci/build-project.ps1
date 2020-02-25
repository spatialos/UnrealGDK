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

# Disable tutorials, otherwise the closing of the window will crash the editor due to some graphic context reason
Add-Content -Path "$unreal_path\Engine\Config\BaseEditorSettings.ini" -Value "`r`n[/Script/IntroTutorials.TutorialStateSettings]`r`nTutorialsProgress=(Tutorial=/Engine/Tutorial/Basics/LevelEditorAttract.LevelEditorAttract_C,CurrentStage=0,bUserDismissed=True)`r`n"

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

    Start-Event "build-android-client" "build-unreal-gdk-example-project-:windows:"

        $unreal_uat_path = "$unreal_path\Engine\Build\BatchFiles\RunUAT.bat"
        $build_server_proc = Start-Process -PassThru -NoNewWindow -FilePath $unreal_uat_path -ArgumentList @(`
            "-ScriptsForProject=`"$test_repo_uproject_path`"", `
            "BuildCookRun", `
            "-nocompileeditor", `
            "-nop4", `
            "-project=`"$test_repo_uproject_path`"", `
            "-cook", `
            "-stage", `
            "-archive", `
            "-archivedirectory=$($exampleproject_home)/cooked-android", `
            "-package", `
            "-clientconfig=Development", `
            "-ue4exe=$unreal_path/Engine/Binaries/Win64/UE4Editor-Cmd.exe", `
            "-pak", `
            "-prereqs", `
            "-nodebuginfo", `
            "-targetplatform=Android", `
            "-cookflavor=Multi", `
            "-build", `
            "-utf8output", `
            "-compile"
        )

        $build_server_handle = $build_server_proc.Handle
        Wait-Process -Id (Get-Process -InputObject $build_server_proc).id

        # Rename the FBuild.tmp back to FBuild.exe clean this up when UNR-2965 is complete
        Rename-Item -Path "C:\Program Files\fastbuild\FBuild.tmp" -NewName "FBuild.exe"

        if ($build_server_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Android Development Client. Error: $($build_server_proc.ExitCode)"
            Throw "Failed to build Android Development Client"
        }
    Finish-Event "build-android-client" "build-unreal-gdk-example-project-:windows:"