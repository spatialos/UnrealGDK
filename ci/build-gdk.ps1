# Expects gdk_home, which is not the GDK location in the engine
param(
  [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine", ## This should ultimately resolve to "C:\b\<number>\UnrealEngine".
  [string] $target_platform = "Win64",
  [string] $build_output_dir
)

pushd "$($gdk_home)"
    Start-Event "build-unreal-gdk-$($target_platform)" "build-gdk"
    pushd "SpatialGDK"
        $gdk_build_proc = Start-Process -PassThru -NoNewWindow -FilePath "$unreal_path\Engine\Build\BatchFiles\RunUAT.bat" -ArgumentList @(`
            "BuildPlugin", `
            "-Plugin=`"$($gdk_home)/SpatialGDK/SpatialGDK.uplugin`"", `
            "-TargetPlatforms=$($target_platform)", `
                # The build output directory here is intentionally meant to be outside both the GDK and the Engine,
                # as apparently there were issues where BuildPlugin would fail when targeting a folder within UnrealEngine
                # (and the gdk is symlinked inside the UnrealEngine) for output.
                # Unreal would find two instances of the plugin during the build process (as it copies the .uplugin to the target folder at the start of the build process)
            "-Package=`"$build_output_dir`"" `
        )
        $gdk_build_handle = $gdk_build_proc.Handle
        Wait-Process -Id (Get-Process -InputObject $gdk_build_proc).id
        if ($gdk_build_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Unreal GDK. Error: $($gdk_build_proc.ExitCode)"
            Throw "Failed to build the Unreal GDK for $($target_platform)"
        }
    Finish-Event "build-unreal-gdk-$($target_platform)" "build-gdk"
    popd
popd
