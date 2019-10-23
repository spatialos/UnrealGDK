# Expects gdk_home
param(
    [string] $build_output_dir,
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine" ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
    [string] $testing_repo_name,
    [string] $testing_repo_branch,
    [string] $testing_repo_url,
    [string] $testing_repo_relative_uproject_path,
)

# Copy the built files back into the SpatialGDK folder, to have a complete plugin
# The trailing \ on the destination path is important!
Copy-Item -Path "$build_output_dir\*" -Destination "$gdk_home\SpatialGDK\" -Recurse -Container -ErrorAction SilentlyContinue

# Clone the testing project, or pull any changes if it has already been cloned
Start-Event "clone-project" "setup-tests"
$project_path = "$unreal_path\Samples\$testing_repo_name"
if (Test-Path $project_path) {
    Write-Log "Project already exists, checking out $($testing_repo_branch) and pulling any changes"
    Git -C $project_path checkout -f $testing_repo_branch
    if(-Not $?) {
        Write-Log "Failed to check out $($testing_repo_name) project from $($testing_repo_url). Error: $($clone_proc.ExitCode)"
        Throw "Failed to clone $($testing_repo_name) project from $($testing_repo_url)"
    }
    Git -C $project_path reset --hard
    if(-Not $?) {
        Write-Log "Failed to check out $($testing_repo_name) project from $($testing_repo_url). Error: $($clone_proc.ExitCode)"
        Throw "Failed to clone $($testing_repo_name) project from $($testing_repo_url)"
    }
    Git -C $project_path pull $testing_repo_url $testing_repo_branch
    if(-Not $?) {
        Write-Log "Failed to check out $($testing_repo_name) project from $($testing_repo_url). Error: $($clone_proc.ExitCode)"
        Throw "Failed to clone $($testing_repo_name) project from $($testing_repo_url)"
    }
  <#  
    $clone_proc = Start-Process -Wait -PassThru -NoNewWindow "git" -ArgumentList @(`
        "-C", `
        "$($project_path)", `
        "checkout", `
        "--force", `
        "$($testing_repo_branch)" `
    )
    $clone_proc = Start-Process -Wait -PassThru -NoNewWindow "git" -ArgumentList @(`
        "-C", `
        "$($project_path)", `
        "reset", `
        "--hard", `
    )
    $clone_proc = Start-Process -Wait -PassThru -NoNewWindow "git" -ArgumentList @(`
        "-C", `
        "$($project_path)", `
        "pull", `
        "$($testing_repo_url)" `
        "$($testing_repo_branch)" `
    ) #>
} else {
    Write-Log "Downloading the $($testing_repo_name) project from $($project_git_source)"
    Git clone -b $testing_repo_branch $testing_repo_url $testing_repo_branch $unreal_path\Samples
    if(-Not $?) {
        Write-Log "Failed to check out $($testing_repo_name) project from $($testing_repo_url). Error: $($clone_proc.ExitCode)"
        Throw "Failed to clone $($testing_repo_name) project from $($testing_repo_url)"
    }
    <#
    $clone_proc = Start-Process -Wait -PassThru -NoNewWindow "git" -ArgumentList @(`
        "clone", `
        "-b" `
        "$($testing_repo_branch)" `
        "$($testing_repo_url)" `
        "$($unreal_path)\Samples" `
    )
    Finish-Event "clone-project" "setup-tests"
    if ($clone_proc.ExitCode -ne 0) {
        Write-Log "Failed to clone $($testing_repo_name) project from $($testing_repo_url). Error: $($clone_proc.ExitCode)"
        Throw "Failed to clone $($testing_repo_name) project from $($testing_repo_url)"
    }#>
}

# The Plugin does not get recognised as an Engine plugin, because we are using a pre-built version of the engine
# copying the plugin into the project's folder bypasses the issue
New-Item -Path "$project_path" -Name "Plugins" -ItemType "directory" -ErrorAction SilentlyContinue
New-Item -ItemType Junction -Name "UnrealGDK" -Path "$project_path\Plugins" -Target "$gdk_home"

# Create the TestResults directory if it does not exist, for storing results
New-Item -Path "$PSScriptRoot" -Name "TestResults" -ItemType "directory" -ErrorAction SilentlyContinue

# Disable tutorials, otherwise the closing of the window will crash the editor due to some graphic context reason
Add-Content -Path "$unreal_path\Engine\Config\BaseEditorSettings.ini" -Value "`r`n[/Script/IntroTutorials.TutorialStateSettings]`r`nTutorialsProgress=(Tutorial=/Engine/Tutorial/Basics/LevelEditorAttract.LevelEditorAttract_C,CurrentStage=0,bUserDismissed=True)`r`n"
