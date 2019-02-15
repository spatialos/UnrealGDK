param(
  [string] $gdk_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the UnrealGDK repo
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

    Start-Event "build-unreal-gdk" "build-unreal-gdk-:windows:"
    pushd "SpatialGDK"
        $build_proc = Start-Process -Wait -PassThru -NoNewWindow -FilePath "$($gdk_home)\Engine\Build\BatchFiles\RunUAT.bat" -ArgumentList @(`
            "BuildPlugin", `
            " -Plugin=`"$($gdk_home)/SpatialGDK/SpatialGDK.uplugin`"", `
            "-TargetPlatforms=Win64", `
            "-Package=`"$gdk_home/SpatialGDK/Intermediate/BuildPackage/Win64`"" `
        )
        if ($build_proc.ExitCode -ne 0) { 
            Write-Log "Failed to build the Unreal GDK. Error: $($build_proc.ExitCode)" 
            Throw "Failed to build the Unreal GDK."  
        }
    popd
    Finish-Event "build-unreal-gdk" "build-unreal-gdk-:windows:"

popd
