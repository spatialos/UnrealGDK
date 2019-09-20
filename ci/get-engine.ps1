pushd "$($gdk_home)"

    # Fetch the version of Unreal Engine we need
    pushd "ci"
        # Allow users to override the engine version if required
        if (Test-Path env:ENGINE_COMMIT_HASH)
        {
            $unreal_version = (Get-Item -Path env:ENGINE_COMMIT_HASH).Value
            Write-Log "Using engine version defined by ENGINE_COMMIT_HASH: $($unreal_version)"
        } else {
            $unreal_version = Get-Content -Path "unreal-engine.version" -Raw
            Write-Log "Using engine version found in unreal-engine.version file: $($unreal_version)"
        }
    popd


    ## Create an UnrealEngine-Cache directory if it doesn't already exist
    New-Item -Name "UnrealEngine-Cache" -ItemType Directory -Force

    pushd "UnrealEngine-Cache"
        Start-Event "download-unreal-engine" "get-unreal-engine"

        if ($unreal_version.StartsWith("HEAD/")) {
            $head_artifacts_gcs_path = "gs://$($gcs_publish_bucket)/$($unreal_version)/UnrealEngine-*"
            $get_head_artifact_result = Call-CaptureOutput "gsutil" -ArgumentList @(`
                "ls", `
                $head_artifacts_gcs_path `
            )

            if ($get_head_artifact_result.ExitCode -ne 0) {
                Write-Log "Could not list artifacts at $($head_artifacts_gcs_path). Error: $($get_head_artifact_result.ExitCode)"
                Throw "Could not list head artifacts"
            }

            $head_artifacts = $get_head_artifact_result.Stdout.Split("`n")
            if ($head_artifacts.Length -eq 0) {
                Write-Log "Could not find a head artifact."
                Throw "Could not find a head artifact"
            }
            if ($head_artifacts.Length -gt 1) {
                Write-Log "Found more than one head artifact."
                Throw "Found more than one head artifact."
            }

            $engine_gcs_path = $head_artifacts[0]
            $version_name = $engine_gcs_path.Split("/")[-1]
            $version_name = $version_name.Split(".")[0]
        } else {
            $version_name = $unreal_version
            $engine_gcs_path = "gs://$($gcs_publish_bucket)/$($version_name).zip"
        }
        Write-Log "Downloading Unreal Engine artifacts from $($engine_gcs_path)"

        $gsu_proc = Start-Process -Wait -PassThru -NoNewWindow "gsutil" -ArgumentList @(`
            "cp", `
            "-n", ` # noclobber
            "$($engine_gcs_path)", `
            "$($version_name).zip" `
        )
        Finish-Event "download-unreal-engine" "get-unreal-engine"
        if ($gsu_proc.ExitCode -ne 0) {
            Write-Log "Failed to download Engine artifacts. Error: $($gsu_proc.ExitCode)"
            Throw "Failed to download Engine artifacts"
        }

        Start-Event "unzip-unreal-engine" "get-unreal-engine"
        Write-Log "Unzipping Unreal Engine"
        $zip_proc = Start-Process -Wait -PassThru -NoNewWindow "7z" -ArgumentList @(`
        "x", `
        "$($version_name).zip", `
        "-o$($version_name)", `
        "-aos" ` # skip existing files
        )
        Finish-Event "unzip-unreal-engine" "get-unreal-engine"
        if ($zip_proc.ExitCode -ne 0) {
            Write-Log "Failed to unzip Unreal Engine. Error: $($zip_proc.ExitCode)"
            Throw "Failed to unzip Unreal Engine."
        }
    popd

    $unreal_path = "$($gdk_home)\UnrealEngine"

    ## Create an UnrealEngine symlink to the correct directory
    Remove-Item $unreal_path -ErrorAction ignore -Recurse -Force
    cmd /c mklink /J $unreal_path "UnrealEngine-Cache\$($version_name)"

    $clang_path = "$($gdk_home)\UnrealEngine\ClangToolchain"
    Write-Log "Setting LINUX_MULTIARCH_ROOT environment variable to $($clang_path)"
    [Environment]::SetEnvironmentVariable("LINUX_MULTIARCH_ROOT", "$($clang_path)", "Machine")
    $Env:LINUX_MULTIARCH_ROOT = "$($clang_path)"

    Start-Event "installing-unreal-engine-prerequisites" "get-unreal-engine"
        # This runs an opaque exe downloaded in the previous step that does *some stuff* that UE needs to occur.
        # Trapping error codes on this is tricky, because it doesn't always return 0 on success, and frankly, we just don't know what it _will_ return.
        Start-Process -Wait -PassThru -NoNewWindow -FilePath "$($unreal_path)/Engine/Extras/Redist/en-us/UE4PrereqSetup_x64.exe" -ArgumentList @(`
            "/quiet" `
        )
    Finish-Event "installing-unreal-engine-prerequisites" "get-unreal-engine"
popd
