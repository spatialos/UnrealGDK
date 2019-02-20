param(
  [string] $gdk_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the UnrealGDK repo
  [string] $gcs_publish_bucket = "io-internal-infra-unreal-artifacts-production/UnrealEngine"
)

$ErrorActionPreference = 'Stop'

function Write-Log() {
  param(
    [string] $msg,
    [Parameter(Mandatory=$false)] [bool] $expand = $false
  )
  if ($expand) {
      Write-Output "+++ $($msg)"
  } else {
      Write-Output "--- $($msg)"
  }
}

function Start-Event() {
    param(
        [string] $event_name,
        [string] $event_parent
    )

    # Start this tracing span.
    Start-Process -NoNewWindow "imp-ci" -ArgumentList @(`
        "events", "new", `
        "--name", "$($event_name)", `
        "--child-of", "$($event_parent)"
    ) | Out-Null

    Write-Log "--- $($event_name)"
}

function Finish-Event() {
    param(
        [string] $event_name,
        [string] $event_parent
    )

    # Emit the end marker for this tracing span.
    Start-Process -NoNewWindow "imp-ci"  -ArgumentList @(`
        "events", "new", `
        "--name", "$($event_name)", `
        "--child-of", "$($event_parent)"
    ) | Out-Null
}

pushd "$($gdk_home)"

    # Fetch the version of Unreal Engine we need
    pushd "ci"
        if (Test-Path env:ENGINE_COMMIT_HASH)
        {
            $unreal_version = Get-Item env:ENGINE_COMMIT_HASH
            Write-Log "Using engine version defined by ENGINE_COMMIT_HASH: $($unreal_version)"
        } else {
            $unreal_version = Get-Content -Path "unreal-engine.version" -Raw
            Write-Log "Using engine version found in unreal-engine.version file: $($unreal_version)"
        }
    popd

    ## Create an UnrealEngine directory if it doesn't already exist
    New-Item -Name "UnrealEngine" -ItemType Directory -Force

    Start-Event "download-unreal-engine" "build-unreal-gdk-:windows:"
    pushd "UnrealEngine"
        Write-Log "Downloading the Unreal Engine artifacts from GCS"
        $gcs_unreal_location = "$($unreal_version).zip"

        $gsu_proc = Start-Process -Wait -PassThru -NoNewWindow "gsutil" -ArgumentList @(`
            "cp", `
            "gs://$($gcs_publish_bucket)/$($gcs_unreal_location)", `
            "$($unreal_version).zip" `
        )
        if ($gsu_proc.ExitCode -ne 0) {
            Write-Log "Failed to download Engine artifacts. Error: $($gsu_proc.ExitCode)"
            Throw "Failed to download Engine artifacts"
        }

        Write-Log "Unzipping Unreal Engine"
        $zip_proc = Start-Process -Wait -PassThru -NoNewWindow "7z" -ArgumentList @(`
        "x", `  
        "$($unreal_version).zip" `    
        )   
        if ($zip_proc.ExitCode -ne 0) { 
            Write-Log "Failed to unzip Unreal Engine. Error: $($zip_proc.ExitCode)" 
            Throw "Failed to unzip Unreal Engine."  
        }
    popd
    Finish-Event "download-unreal-engine" "build-unreal-gdk-:windows:"

    $unreal_path = "$($gdk_home)\UnrealEngine"
    Write-Log "Setting UNREAL_HOME environment variable to $($unreal_path)"
    [Environment]::SetEnvironmentVariable("UNREAL_HOME", "$($unreal_path)", "Machine")

    $clang_path = "$($gdk_home)\UnrealEngine\ClangToolchain"
    Write-Log "Setting LINUX_MULTIARCH_ROOT environment variable to $($clang_path)"
    [Environment]::SetEnvironmentVariable("LINUX_MULTIARCH_ROOT", "$($clang_path)", "Machine")

popd