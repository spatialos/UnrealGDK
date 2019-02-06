param(
  [string] $gdk_home = "$($PSScriptRoot)/..", ## The root of the UnrealGDK repo
  [string] $gcs_publish_bucket = "io-internal-infra-unreal-artifacts-production"
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


Write-Output "Starting Unreal GDK build pipeline.."

pushd "$($gdk_home)"

    # Fetch the version of Unreal Engine we need
    pushd "ci"
        $unreal_version = Get-Content -Path "unreal-engine.version" -Raw
        Write-Log "Using Unreal Engine version $($unreal_version)"
    popd

    Write-Log "Create an UnrealEngine directory if it doesn't already exist"
    new-item -Name "UnrealEngine" -itemtype directory

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
        "e", `
        "$($unreal_version).zip", `
        "-aoa" `
        )
        if ($zip_proc.ExitCode -ne 0) {
            Write-Log "Failed to unzip Unreal Engine. Error: $($zip_proc.ExitCode)"
            Throw "Failed to unzip Unreal Engine."
        }

    popd

    #[Environment]::SetEnvironmentVariable("UNREAL_HOME", "$($gdk_home)/UnrealEngine", "Machine")

  # Run the Setup.bat file located in the root
  <#Write-Log "Setup Unreal GDK dependencies" -Expand $true
  Start-Process -Wait -PassThru -NoNewWindow -FilePath "$($gdk_home)\Setup.bat"
  if ($LASTEXITCODE -ne 0) {
      Write-Log "Failed to install Unreal GDK dependencies.  Error code $($LASTEXITCODE)"
      Throw "Failed to install Unreal GDK dependencies"
  }

  pushd "SpatialGDK"
  
    # Finally, build the Unreal GDK 
    Write-Log "Build Unreal GDK"
    Start-Process -Wait -PassThru -NoNewWindow -FilePath "$($UNREAL_HOME)\Engine\Build\BatchFiles\RunUAT.bat" -ArgumentList @(`
        "BuildPlugin", `
        " -Plugin=`"$PWD/SpatialGDK.uplugin`"", `
        "-TargetPlatforms=Win64", `
        "-Package=`"$PWD/Intermediate/BuildPackage/Win64`"" `
    )

  popd#>

popd