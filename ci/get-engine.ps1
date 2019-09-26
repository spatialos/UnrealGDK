param(
    # Note: this directory is outside the build directory and will not get automatically cleaned up from agents unless agents are restarted.
    [string] $engine_cache_directory = "$($pwd.drive.root)UnrealEngine-Cache",
    # Unreal path is a symlink to a specific Engine version located in Engine cache directory.
    [string] $unreal_path = "$($gdk_home)\UnrealEngine"
)

pushd "$($gdk_home)"

    # Fetch the version of Unreal Engine we need
    pushd "ci"
        # Allow users to override the engine version if required
        if (Test-Path env:ENGINE_COMMIT_HASH)
        {
            $unreal_version = (Get-Item -Path env:ENGINE_COMMIT_HASH).Value
            Write-Log "Using engine version defined by ENGINE_COMMIT_HASH: $($unreal_version)"
        } else {
            # Read Engine version from the file and trim any trailing white spaces and new lines.
            $unreal_version = (Get-Content -Path "unreal-engine.version" -Raw).Trim()
            Write-Log "Using engine version found in unreal-engine.version file: $($unreal_version)"
        }
    popd

    ## Create an UnrealEngine-Cache directory if it doesn't already exist.
    New-Item -ItemType Directory -Path $engine_cache_directory -Force

    pushd $engine_cache_directory
        Start-Event "download-unreal-engine" "get-unreal-engine"

        $engine_gcs_path = "gs://$($gcs_publish_bucket)/$($unreal_version).zip"
        Write-Log "Downloading Unreal Engine artifacts from $($engine_gcs_path)"

        $gsu_proc = Start-Process -Wait -PassThru -NoNewWindow "gsutil" -ArgumentList @(`
            "cp", `
            "-n", ` # noclobber
            "$($engine_gcs_path)", `
            "$($unreal_version).zip" `
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
        "$($unreal_version).zip", `
        "-o$($unreal_version)", `
        "-aos" ` # skip existing files
        )
        Finish-Event "unzip-unreal-engine" "get-unreal-engine"
        if ($zip_proc.ExitCode -ne 0) {
            Write-Log "Failed to unzip Unreal Engine. Error: $($zip_proc.ExitCode)"
            Throw "Failed to unzip Unreal Engine."
        }
    popd

    ## Create an UnrealEngine symlink to the correct directory
    Remove-Item $unreal_path -ErrorAction ignore -Recurse -Force
    cmd /c mklink /J $unreal_path "$engine_cache_directory\$($unreal_version)"

    $clang_path = "$unreal_path\ClangToolchain"
    Write-Log "Setting LINUX_MULTIARCH_ROOT environment variable to $($clang_path)"
    [Environment]::SetEnvironmentVariable("LINUX_MULTIARCH_ROOT", "$($clang_path)", "Machine")
    $Env:LINUX_MULTIARCH_ROOT = "$($clang_path)"

    Start-Event "installing-unreal-engine-prerequisites" "get-unreal-engine"
        # This runs an opaque exe downloaded in the previous step that does *some stuff* that UE needs to occur.
        # Trapping error codes on this is tricky, because it doesn't always return 0 on success, and frankly, we just don't know what it _will_ return.
        # Note: this fails to install .NET framework, but it's probably fine, as it's set up on Unreal build agents already (check gdk-for-unreal.build-capability/roles/gdk_for_unreal_choco/tasks/Windows.yml)
        Start-Process -Wait -PassThru -NoNewWindow -FilePath "$($unreal_path)/Engine/Extras/Redist/en-us/UE4PrereqSetup_x64.exe" -ArgumentList @(`
            "/quiet" `
        )
    Finish-Event "installing-unreal-engine-prerequisites" "get-unreal-engine"
popd
