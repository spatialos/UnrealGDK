. "$PSScriptRoot\common.ps1"

pushd "$($gdk_home)"

    Start-Event "build-unreal-gdk" "build-unreal-gdk-:windows:"
    pushd "SpatialGDK"
        $win_build_proc = Start-Process -PassThru -NoNewWindow -FilePath "$($gdk_home)\UnrealEngine\Engine\Build\BatchFiles\RunUAT.bat" -ArgumentList @(`
            "BuildPlugin", `
            " -Plugin=`"$($gdk_home)/SpatialGDK/SpatialGDK.uplugin`"", `
            "-TargetPlatforms=Win64", `
            "-Package=`"$($gdk_home)/SpatialGDK/Intermediate/BuildPackage/Win64`"" `
        )
        $win_build_handle = $win_build_proc.Handle
        Wait-Process -Id (Get-Process -InputObject $win_build_proc).id
        if ($win_build_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Unreal GDK. Error: $($win_build_proc.ExitCode)"
            Throw "Failed to build the Unreal GDK for Windows"
        }
    Finish-Event "build-unreal-gdk" "build-unreal-gdk-:windows:"
    popd
popd
