param(
    [string] $project_clone_path,
    [string] $unreal_path,
    [string] $gdk_build_path
)

pushd $project_clone_path
    if (Test-Path "UnrealGDKExampleProject") {
        pushd "UnrealGDKExampleProject"
            Start-Process -Wait -PassThru -NoNewWindow "git" -ArgumentList @("pull")
        popd
    } else {
        $clone_proc = Start-Process -Wait -PassThru -NoNewWindow "git" -ArgumentList @(`
            "clone", `
            "https://github.com/spatialos/UnrealGDKExampleProject"
        )
        
        if ($clone_proc.ExitCode -ne 0) {
            Write-Log "Could not clone example project, error: $($clone_proc.ExitCode)"
            Throw "Could not clone example project"
        }    
    }
popd

    # copy the built plugin into the engine
Copy-Item $gdk_build_path "$($unreal_path)\Engine\Plugins\SpatialGDK" -Recurse
Write-Log "plugins contents: $(Get-ChildItem $($unreal_path)\Engine\Plugins)"
Write-Log "spatialgdk contents: $(Get-ChildItem $($unreal_path)\Engine\Plugins\SpatialGDK)"