using System;
using System.Text;
using System.IO;
using System.Linq;

namespace Improbable
{
    public static class Build
    {
        private const string ShellScript =
@"#!/bin/bash
NEW_USER=unrealworker
WORKER_ID=$1
WORKER_NAME=$2
shift 2

# 2>/dev/null silences errors by redirecting stderr to the null device. This is done to prevent errors when a machine attempts to add the same user more than once.
useradd $NEW_USER -m -d /improbable/logs/UnrealWorker/Logs 2>/dev/null
chown -R $NEW_USER:$NEW_USER $(pwd)
chmod -R o+rw /improbable/logs
SCRIPT=""$(pwd)/${WORKER_NAME}Server.sh""
chmod +x $SCRIPT
gosu $NEW_USER ""${SCRIPT}"" ""$@"" 2> >(grep -v xdg-user-dir >&2)`";

        private const string  runEditorScript =
@"setlocal ENABLEDELAYEDEXPANSION
%%UNREAL_HOME%%\Engine\Binaries\Win64\UE4Editor.exe ""{0}"" %%*
exit /b !ERRORLEVEL!
";
        public static void Main(string[] args)
        {
            var runCodegen = args.Count(arg => arg.ToLowerInvariant() == "--skip-codegen") == 0;
            var help = args.Count(arg => arg == "/?" || arg.ToLowerInvariant() == "--help") > 0;

            var exitCode = 0;
            if(args.Length != 5)
            {
                help = true;
                exitCode = 1;
                Console.Error.WriteLine("Path to uproject file is required.");
            }

            if(help)
            {
                Console.WriteLine("Usage: <GameName> <Platform> <Configuration> <game.uproject> [flags]");
                Console.WriteLine("Flags:");
                Console.WriteLine("\t--skip-codegen  : Don't run the code generator.");

                Environment.Exit(exitCode);
            }

            var gameName = args[1];
            var platform = args[2];
            var configuration = args[3];
            var projectFile = Path.GetFullPath(args[4]);

            if(runCodegen)
            {
                Codegen.Main(args);
            }
            else
            {
                Common.WriteHeading("Skipping code generation.");
            }

            var stagingDir = Path.GetFullPath(Path.Combine("spatial", "build", "unreal"));
            var outputDir = Path.GetFullPath(Path.Combine("spatial", "build", "assembly", "worker"));

            switch(gameName)
            {
                case "SampleGameEditor":
                    Common.WriteHeading(" > Building editor for use as a managed worker.");
                    var windowsEditorPath = Path.Combine(stagingDir, "WindowsEditor");

                    if(!Directory.Exists(windowsEditorPath))
                    {
                        Directory.CreateDirectory(windowsEditorPath);
                    }

                    Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\Build.bat", new [] {
                        "SampleGameEditor",
                        "Win64",
                        configuration,
                        Quote(projectFile)
                    });

                    // Write a simple batch file to launch the Editor.
                    File.WriteAllText(Path.Combine(windowsEditorPath, "StartServer.bat"), string.Format(runEditorScript, projectFile), Encoding.UTF8);

                    // The runtime currently requires all workers to be in zip files. Zip the batch file.
                    Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new [] {
                        "ZipUtils",
                        "-add=" + Quote(windowsEditorPath),
                        "-archive=" + Quote(Path.Combine(outputDir, "UnrealEditor@Windows.zip")),
                    });
                    break;

                case "SampleGame":
                    Common.WriteHeading(" > Building client.");
                    Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new [] {
                        "BuildCookRun",
                        "-build",
                        "-project=" + Quote(projectFile),
                        "-noP4",
                        "-clientconfig=" + configuration,
                        "-serverconfig=" + configuration,
                        "-utf8output",
                        "-compile",
                        "-cook",
                        "-stage",
                        "-package",
                        "-unversioned",
                        "-compressed",
                        "-stagingdirectory=" + Quote(stagingDir),
                        "-stdout",
                        "-FORCELOGFLUSH",
                        "-CrashForUAT",
                        "-unattended",
                        "-fileopenlog",
                        "-SkipCookingEditorContent",
                        "-platform=" + "Win64",
                        "-targetplatform=" + "Win64",
                        "-allmaps",
                    });

                    var windowsNoEditorPath = Path.Combine(stagingDir, "WindowsNoEditor");
                    Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new [] {
                        "ZipUtils",
                        "-add=" + Quote(windowsNoEditorPath),
                        "-archive=" + Quote(Path.Combine(outputDir, "UnrealClient@Windows.zip")),
                    });
                    break;

                case "SampleGameServer":
                    Common.WriteHeading(" > Building Linux worker.");
                    Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new [] {
                        "BuildCookRun",
                        "-build",
                        "-project=" + Quote(projectFile),
                        "-noP4",
                        "-clientconfig=" + configuration,
                        "-serverconfig=" + configuration,
                        "-utf8output",
                        "-compile",
                        "-cook",
                        "-stage",
                        "-package",
                        "-unversioned",
                        "-compressed",
                        "-stagingdirectory=" + Quote(stagingDir),
                        "-stdout",
                        "-FORCELOGFLUSH",
                        "-CrashForUAT",
                        "-unattended",
                        "-fileopenlog",
                        "-SkipCookingEditorContent",
                        "-allmaps",
                        "-server",
                        "-serverplatform=" + "Linux",
                        "-noclient",
                    });

                    var linuxServerPath = Path.Combine(stagingDir, "LinuxServer");
                    File.WriteAllText(Path.Combine(linuxServerPath, "StartServer.sh"), ShellScript, Encoding.UTF8);

                    Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new [] {
                        "ZipUtils",
                        "-add=" + Quote(linuxServerPath),
                        "-archive=" + Quote(Path.Combine(outputDir, "UnityWorker@Linux.zip"))
                    });
                    break;
                default:
                    // Pass-through to Unreal's Build.bat.
                    Common.WriteHeading($" > Building ${gameName}.");
                    Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\Build.bat", new [] {
                        gameName,
                        platform,
                        configuration,
                        Quote(projectFile)
                    });
                    break;
            }
        }

        private static string Quote(string toQuote)
        {
            return string.Format("\"{0}\"", toQuote);
        }
    }
}
