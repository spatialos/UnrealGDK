param(
  [string] $target_platform = "Win64"
)

pushd "$($gdk_home)"

    Start-Event "build-unreal-gdk-$($target_platform)" "build-gdk"
    pushd "SpatialGDK"
        $gdk_build_proc = Start-Process -PassThru -NoNewWindow -FilePath "$($gdk_home)\UnrealEngine\Engine\Build\BatchFiles\RunUAT.bat" -ArgumentList @(`
            "BuildPlugin", `
            " -Plugin=`"$($gdk_home)/SpatialGDK/SpatialGDK.uplugin`"", `
            "-TargetPlatforms=$($target_platform)", `
            "-Package=`"$($gdk_home)/SpatialGDK/Intermediate/BuildPackage/Win64`"" `
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
