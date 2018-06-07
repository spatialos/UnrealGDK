using System;
using System.IO;
using System.Linq;
using System.Text;

namespace Improbable
{
    public static class Build
    {
        private const string UnrealWorkerShellScript =
            @"#!/bin/bash
NEW_USER=unrealworker
WORKER_ID=$1
WORKER_NAME=$2
shift 2

# 2>/dev/null silences errors by redirecting stderr to the null device. This is done to prevent errors when a machine attempts to add the same user more than once.
useradd $NEW_USER -m -d /improbable/logs/UnrealWorker/Logs 2>/dev/null
chown -R $NEW_USER:$NEW_USER $(pwd) 2>/dev/null
chmod -R o+rw /improbable/logs 2>/dev/null
SCRIPT=""$(pwd)/${WORKER_NAME}Server.sh""
chmod +x $SCRIPT
gosu $NEW_USER ""${SCRIPT}"" ""$@"" 2> >(grep -v xdg-user-dir >&2)`";

        private const string RunEditorScript =
            @"setlocal ENABLEDELAYEDEXPANSION
%%UNREAL_HOME%%\Engine\Binaries\Win64\UE4Editor.exe ""{0}"" %%*
exit /b !ERRORLEVEL!
";

        public static void Main(string[] args)
        {
            var runCodegen = args.Count(arg => arg.ToLowerInvariant() == "--skip-codegen") == 0;
            var help = args.Count(arg => arg == "/?" || arg.ToLowerInvariant() == "--help") > 0;

            var exitCode = 0;
            if (args.Length < 4 && !help)
            {
                help = true;
                exitCode = 1;
                Console.Error.WriteLine("Path to uproject file is required.");
            }

            if (help)
            {
                Console.WriteLine("Usage: <GameName> <Platform> <Configuration> <game.uproject> [flags]");
                Console.WriteLine("Flags:");
                Console.WriteLine("\t--skip-codegen  : Don't run the code generator.");

                Environment.Exit(exitCode);
            }

            var gameName = args[0];
            var platform = args[1];
            var configuration = args[2];
            var projectFile = Path.GetFullPath(args[3]);

            if (runCodegen)
            {
                Common.WriteHeading("Generating code...");
                Codegen.Main(args);
            }
            else
            {
                Common.WriteHeading("Skipping code generation.");
            }

            var stagingDir = Path.GetFullPath(Path.Combine("../spatial", "build", "unreal"));
            var outputDir = Path.GetFullPath(Path.Combine("../spatial", "build", "assembly", "worker"));
            var baseGameName = Path.GetFileNameWithoutExtension(projectFile);

            if (gameName == baseGameName + "Editor")
            {
                Common.WriteHeading(" > Building Editor for use as a managed worker.");

                Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\Build.bat", new[]
                {
                    gameName,
                    platform,
                    configuration,
                    Quote(projectFile)
                });

                var windowsEditorPath = Path.Combine(stagingDir, "WindowsEditor");
                if (!Directory.Exists(windowsEditorPath))
                {
                    Directory.CreateDirectory(windowsEditorPath);
                }

                // Write a simple batch file to launch the Editor as a managed worker.
                File.WriteAllText(Path.Combine(windowsEditorPath, "StartEditor.bat"),
                    string.Format(RunEditorScript, projectFile), Encoding.UTF8);

                // The runtime currently requires all workers to be in zip files. Zip the batch file.
                Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new[]
                {
                    "ZipUtils",
                    "-add=" + Quote(windowsEditorPath),
                    "-archive=" + Quote(Path.Combine(outputDir, "UnrealEditor@Windows.zip")),
                });
            }
            else if (gameName == baseGameName)
            {
                Common.WriteHeading(" > Building client.");
                Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new[]
                {
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
                    "-platform=" + platform,
                    "-targetplatform=" + platform,
                    "-allmaps",
                });

                var windowsNoEditorPath = Path.Combine(stagingDir, "WindowsNoEditor");
                Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new[]
                {
                    "ZipUtils",
                    "-add=" + Quote(windowsNoEditorPath),
                    "-archive=" + Quote(Path.Combine(outputDir, "UnrealClient@Windows.zip")),
                });
            }
            else if (gameName == baseGameName + "Server")
            {
                Common.WriteHeading(" > Building worker.");
                Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new[]
                {
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
                    "-serverplatform=" + platform,
                    "-noclient",
                });

                // Write out the wrapper shell script to work around issues between UnrealEngine and our cloud Linux environments.
                var linuxServerPath = Path.Combine(stagingDir, "LinuxServer");
                File.WriteAllText(Path.Combine(linuxServerPath, "StartWorker.sh"), UnrealWorkerShellScript,
                    Encoding.UTF8);

                Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new[]
                {
                    "ZipUtils",
                    "-add=" + Quote(linuxServerPath),
                    "-archive=" + Quote(Path.Combine(outputDir, "UnrealWorker@Linux.zip"))
                });
            }
            else
            {
                // Pass-through to Unreal's Build.bat.
                Common.WriteHeading($" > Building ${gameName}.");
                Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\Build.bat", new[]
                {
                    gameName,
                    platform,
                    configuration,
                    Quote(projectFile)
                });
            }
        }

        private static string Quote(string toQuote)
        {
            return $"\"{toQuote}\"";
        }
    }
}
