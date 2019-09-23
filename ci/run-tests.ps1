param(
    [string] $gdk_home = (get-item "$($PSScriptRoot)").parent.FullName ## The root of the UnrealGDK repo
)
.\Engine\Binaries\Win64\UE4Editor.exe "C:\Users\michelefabris\git\UnrealGDKExampleProject\Game\GDKShooter.uproject" -ExecCmds="Automation runtests SpatialGDK" -log  -Unattended -NullRHI -TestExit="Automation Test Queue Em
pty" -ReportOutputPath="C:\Users\michelefabris\GdkTestReport" -Log -Log=RunTests.log