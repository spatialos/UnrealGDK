# Expects gdk_home, which is not the GDK location in the engine
param(
  [string] $ue_home = "$($pwd.drive.root)UnrealEngine",
  [string] $target_platform = "Win64"
)

pushd "$($gdk_home)"
    Start-Event "build-unreal-gdk-$($target_platform)" "build-gdk"
    pushd "SpatialGDK"
        $gdk_build_proc = Start-Process -PassThru -NoNewWindow -FilePath "$ue_home\Engine\Build\BatchFiles\RunUAT.bat" -ArgumentList @(`
            "BuildPlugin", `
            "-Plugin=`"$($gdk_home)/SpatialGDK/SpatialGDK.uplugin`"", `
            "-TargetPlatforms=$($target_platform)", `
                # Since we symlink the gdk into the engine, we have to put the build output outside of the GDK, 
                # otherwise Unreal will find two instances of the plugin during the build process (It copies the .uplugin to the target folder at the start of the build process)
            "-Package=`"C:\SpatialGDKBuild`"" ` # TODO
        $gdk_build_handle = $gdk_build_proc.Handle
        Wait-Process -Id (Get-Process -InputObject $gdk_build_proc).id
        if ($gdk_build_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Unreal GDK. Error: $($gdk_build_proc.ExitCode)"
            Throw "Failed to build the Unreal GDK for $($target_platform)"
        }
    Finish-Event "build-unreal-gdk-$($target_platform)" "build-gdk"
    popd
popd
