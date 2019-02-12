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

    # Fetch the version of Unreal Engine we need
    pushd "ci"
        $unreal_version = Get-Content -Path "unreal-engine.version" -Raw
        Write-Log "Using Unreal Engine version: $($unreal_version)"
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

    ## THIS REPLACES THE OLD SETUP.BAT SCRIPT

    # Setup variables
    $pinned_core_sdk_version = Get-Content -Path "$($gdk_home)\SpatialGDK\Extras\core-sdk.version" -Raw
    $build_dir = "$($gdk_home)\SpatialGDK\Build"
    $core_sdk_dir = "$($build_dir)\core_sdk"
    $worker_sdk_dir = "$($gdk_home)\SpatialGDK\Source\SpatialGDK\Public\WorkerSDK"
    $worker_sdk_dir_old = "$($gdk_home)\SpatialGDK\Source\Public\WorkerSdk"
    $binaries_dir = "$($gdk_home)\SpatialGDK\Binaries\ThirdParty\Improbable"

    Write-Log "Creating folders.."
    New-Item -Path "$($worker_sdk_dir)" -ItemType Directory -Force
    New-Item -Path "$($core_sdk_dir)\schema" -ItemType Directory -Force
    New-Item -Path "$($core_sdk_dir)\tools" -ItemType Directory -Force
    New-Item -Path "$($core_sdk_dir)\worker_sdk" -ItemType Directory -Force
    New-Item -Path "$($binaries_dir)" -ItemType Directory -Force


    Start-Event "download-spatial-packages" "build-unreal-gdk-:windows:"
    Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
        "package", `
        "retrieve", `
        "tools", `
        "schema_compiler-x86_64-win32", `
        "$($pinned_core_sdk_version)", `
        "$($core_sdk_dir)\tools\schema_compiler-x86_64-win32.zip" `
    )
    Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
        "package", `
        "retrieve", `
        "schema", `
        "standard_library", `
        "$($pinned_core_sdk_version)", `
        "$($core_sdk_dir)\schema\standard_library.zip" `
    )
    Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
        "package", `
        "retrieve", `
        "worker_sdk", `
        "c-dynamic-x86-msvc_md-win32", `
        "$($pinned_core_sdk_version)", `
        "$($core_sdk_dir)\worker_sdk\c-dynamic-x86-msvc_md-win32.zip" `
    )

    Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
        "package", `
        "retrieve", `
        "worker_sdk", `
        "c-dynamic-x86_64-msvc_md-win32", `
        "$($pinned_core_sdk_version)", `
        "$($core_sdk_dir)\worker_sdk\c-dynamic-x86_64-msvc_md-win32.zip" `
    )
    Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
        "package", `
        "retrieve", `
        "worker_sdk", `
        "c-dynamic-x86_64-gcc_libstdcpp-linux", `
        "$($pinned_core_sdk_version)", `
        "$($core_sdk_dir)\worker_sdk\c-dynamic-x86_64-gcc_libstdcpp-linux.zip" `
    )
    Finish-Event "download-spatial-packages" "build-unreal-gdk-:windows:"


    Start-Event "extract-spatial-packages" "build-unreal-gdk-:windows:"
    Expand-Archive -Path "$($core_sdk_dir)\tools\schema_compiler-x86_64-win32.zip" -DestinationPath "$($binaries_dir)\Programs\" -Force
    Expand-Archive -Path "$($core_sdk_dir)\schema\standard_library.zip" -DestinationPath "$($binaries_dir)\Programs\schema\" -Force
    Expand-Archive -Path "$($core_sdk_dir)\worker_sdk\c-dynamic-x86-msvc_md-win32.zip" -DestinationPath "$($binaries_dir)\Win32\" -Force
    Expand-Archive -Path "$($core_sdk_dir)\worker_sdk\c-dynamic-x86_64-msvc_md-win32.zip" -DestinationPath "$($binaries_dir)\Win64\" -Force
    Expand-Archive -Path "$($core_sdk_dir)\worker_sdk\c-dynamic-x86_64-gcc_libstdcpp-linux.zip" -DestinationPath "$($binaries_dir)\Linux\" -Force
    Finish-Event "extract-spatial-packages" "build-unreal-gdk-:windows:"


    # Copy from binaries_dir
    Copy-Item "$($binaries_dir)\Win64\include\*" "$($worker_sdk_dir)\" -Force -Recurse

    Write-Log "Fetch MSBUILD_EXE location"
    $msbuild_exe = "${env:ProgramFiles(x86)}\MSBuild\14.0\bin\MSBuild.exe"


    Start-Event "build-utilities" "build-unreal-gdk-:windows:"
    #%MSBUILD_EXE% /nologo /verbosity:minimal .\SpatialGDK\Build\Programs\Improbable.Unreal.Scripts\Improbable.Unreal.Scripts.sln /property:Configuration=Release
    $msbuild_proc = Start-Process -Wait -PassThru -NoNewWindow -FilePath "$($msbuild_exe)" -ArgumentList @(`
        "/nologo", `
        "SpatialGDK\Build\Programs\Improbable.Unreal.Scripts\Improbable.Unreal.Scripts.sln", `
        "/property:Configuration=Release" `
    )
    if ($msbuild_proc.ExitCode -ne 0) { 
        Write-Log "Failed to build utilities. Error: $($msbuild_proc.ExitCode)" 
        Throw "Failed to build utilities."  
    }
    Finish-Event "build-utilities" "build-unreal-gdk-:windows:"


    Start-Event "build-unreal-gdk" "build-unreal-gdk-:windows:"
    pushd "SpatialGDK"
        $build_proc = Start-Process -Wait -PassThru -NoNewWindow -FilePath "$($unreal_path)\Engine\Build\BatchFiles\RunUAT.bat" -ArgumentList @(`
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