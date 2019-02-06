param(
  [string] $gdk_home = "$($PSScriptRoot)/.." ## The root of the UnrealGDK repo
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

  pushd "SpatialGDK/Extras"

    # Make sure we've set the UNREAL_VERSION and UNREAL_HOME environment variables
    Write-Log "Setup Unreal GDK variables" -Expand $true
    UNREAL_VERSION=$(cat unreal-engine.version)
    ## UNREAL_HOME=C:/Unreal/UnrealEngine-${UNREAL_VERSION} ## TODO - We cannot depend on this naming!! Fix this!

  popd

  # Run the Setup.bat file located in the root
  Write-Log "Setup Unreal GDK dependencies" -Expand $true
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

  popd

popd