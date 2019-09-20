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

    Write-Log "$($event_name)"
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

function Call-CaptureOutput() {
    param(
        [string] $FilePath,
        [string[]] $ArgumentList
    )

    $out_file = New-TemporaryFile
    $proc_handle = Start-Process -Wait -PassThru -NoNewWindow $FilePath -ArgumentList $ArgumentList -RedirectStandardOutput $out_file.FullName
    $stdout = Get-Content $out_file.FullName
    Remove-Item $out_file

        # return an object with exit code and stdout string
    [PsCustomObject]@{
        ExitCode = $proc_handle.ExitCode;
        Stdout = $stdout;
    }
}

$ErrorActionPreference = 'Stop'
