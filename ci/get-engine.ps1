param(
    # Note: this directory is outside the build directory and will not get automatically cleaned up from agents unless agents are restarted.
    [string] $engine_cache_directory = "$($pwd.drive.root)UnrealEngine-Cache",

    # Unreal path is a symlink to a specific Engine version located in Engine cache directory. This should ultimately resolve to "C:\b\<number>\UnrealEngine".
    [string] $unreal_path = "$((Get-Item `"$($PSScriptRoot)`").parent.parent.FullName)\UnrealEngine",
    
    [string] $gcs_publish_bucket = "io-internal-infra-unreal-artifacts-production/UnrealEngine"
)

pushd "$($gdk_home)"

    # Fetch the version of Unreal Engine we need
    pushd "ci"
        # Allow overriding the engine version if required
        if (Test-Path env:ENGINE_COMMIT_HASH) {
            $version_description = (Get-Item -Path env:ENGINE_COMMIT_HASH).Value
            Echo "Using engine version defined by ENGINE_COMMIT_HASH: $($version_description)"
        } else {
            # Read Engine version from the file and trim any trailing white spaces and new lines.
            $version_description = Get-Content -Path "unreal-engine.version" -First 1
            Echo "Using engine version found in unreal-engine.version file: $($version_description)"
        }

        # Check if we are using a 'floating' engine version, meaning that we want to get the latest built version of the engine on some branch
        # This is specified by putting "HEAD name/of-a-branch" in the unreal-engine.version file
        # If so, retrieve the version of the latest build from GCS, and use that going forward.
        $head_version_prefix = "HEAD " 
        if ($version_description.StartsWith($head_version_prefix)) {
            $version_branch = $version_description.Remove(0, $head_version_prefix.Length) # Remove the prefix to just get the branch name
            $version_branch = $version_branch.Replace("/", "_") # Replace / with _ since / is treated as the folder seperator in GCS

            # Download the head pointer file for the given branch, which contains the latest built version of the engine from that branch
            $head_pointer_gcs_path = "gs://$($gcs_publish_bucket)/HEAD/$($version_branch).version"
            $unreal_version = $(gsutil cp $head_pointer_gcs_path -) # the '-' at the end instructs gsutil to download the file and output the contents to stdout
        } else {
            $unreal_version = $version_description
        }
    popd
    
    ## Create an UnrealEngine-Cache directory if it doesn't already exist.
    New-Item -ItemType Directory -Path $engine_cache_directory -Force

    pushd $engine_cache_directory
        Start-Event "download-unreal-engine" "get-unreal-engine"

        $engine_gcs_path = "gs://$($gcs_publish_bucket)/$($unreal_version).zip"
        Echo "Downloading Unreal Engine artifacts version $unreal_version from $($engine_gcs_path)"

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
    New-Item -ItemType Junction -Path "$unreal_path" -Target "$engine_cache_directory\$($unreal_version)"

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
