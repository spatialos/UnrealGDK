. "$PSScriptRoot\common.ps1"

pushd "$($gdk_home)"

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
    New-Item -Path "$($binaries_dir)\Programs" -ItemType Directory -Force

    # Download GDK dependencies through the spatial package manager 
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

    Start-Event "build-utilities" "build-unreal-gdk-:windows:"
    $msbuild_proc = Start-Process -PassThru -NoNewWindow -FilePath "$($msbuild_exe)" -ArgumentList @(`
        "/nologo", `
        "SpatialGDK\Build\Programs\Improbable.Unreal.Scripts\Improbable.Unreal.Scripts.sln", `
        "/property:Configuration=Release" `
    )

    # Note: holding on to a handle solves an intermittent issue when waiting on the process id
	$msbuild_handle = $msbuild_proc.Handle
    Wait-Process -Id (Get-Process -InputObject $msbuild_proc).id
    if ($msbuild_proc.ExitCode -ne 0) { 
        Write-Log "Failed to build utilities. Error: $($msbuild_proc.ExitCode)" 
        Throw "Failed to build utilities."  
    }
    Finish-Event "build-utilities" "build-unreal-gdk-:windows:"
popd
