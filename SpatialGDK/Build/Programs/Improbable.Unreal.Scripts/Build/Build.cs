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
shift 1

# 2>/dev/null silences errors by redirecting stderr to the null device. This is done to prevent errors when a machine attempts to add the same user more than once.
useradd $NEW_USER -m -d /improbable/logs/UnrealWorker/Logs 2>/dev/null
chown -R $NEW_USER:$NEW_USER $(pwd) 2>/dev/null
chmod -R o+rw /improbable/logs 2>/dev/null
SCRIPT=""$(pwd)/{0}Server.sh""
chmod +x $SCRIPT
gosu $NEW_USER ""${{SCRIPT}}"" ""$@"" 2> >(grep -v xdg-user-dir >&2)";

        private const string RunEditorScript =
            @"setlocal ENABLEDELAYEDEXPANSION
%UNREAL_HOME%\Engine\Binaries\Win64\UE4Editor.exe ""{0}"" %*
exit /b !ERRORLEVEL!
";

        public static void Main(string[] args)
        {
            var help = args.Count(arg => arg == "/?" || arg.ToLowerInvariant() == "--help") > 0;
            var noCompile = false;

            var exitCode = 0;
            if (args.Length < 4 && !help)
            {
                help = true;
                exitCode = 1;
                Console.Error.WriteLine("Path to uproject file is required.");
            }
            else if (args.Length > 4)
            {
                if (args[4].CompareTo("-nocompile") != 0)
                {
                    help = true;
                    exitCode = 1;
                }
                else
                {
                    noCompile = true;
                }
            }

            if (help)
            {
                Console.WriteLine("Usage: <GameName> <Platform> <Configuration> <game.uproject> [-nocompile]");

                Environment.Exit(exitCode);
            }

            var gameName = args[0];
            var platform = args[1];
            var configuration = args[2];
            var projectFile = Path.GetFullPath(args[3]);

            var stagingDir = Path.GetFullPath(Path.Combine("../spatial", "build", "unreal"));
            var outputDir = Path.GetFullPath(Path.Combine("../spatial", "build", "assembly", "worker"));
            var baseGameName = Path.GetFileNameWithoutExtension(projectFile);

            if (gameName == baseGameName + "Editor")
            {
                if (noCompile)
                {
                    Common.WriteHeading(" > Using precompiled Editor for use as a managed worker.");
                }
                else
                {
                    Common.WriteHeading(" > Building Editor for use as a managed worker.");

                    Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\Build.bat", new[]
                    {
                        gameName,
                        platform,
                        configuration,
                        Quote(projectFile)
                    });
                }

                var windowsEditorPath = Path.Combine(stagingDir, "WindowsEditor");
                if (!Directory.Exists(windowsEditorPath))
                {
                    Directory.CreateDirectory(windowsEditorPath);
                }

                // Write a simple batch file to launch the Editor as a managed worker.
                File.WriteAllText(Path.Combine(windowsEditorPath, "StartEditor.bat"),
                    string.Format(RunEditorScript, projectFile), new UTF8Encoding(false));

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
                    noCompile ? "-nobuild" : "-build",
                    noCompile ? "-nocompile" : "-compile",
                    "-project=" + Quote(projectFile),
                    "-noP4",
                    "-clientconfig=" + configuration,
                    "-serverconfig=" + configuration,
                    "-utf8output",
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
                    noCompile ? "-nobuild" : "-build",
                    noCompile ? "-nocompile" : "-compile",
                    "-project=" + Quote(projectFile),
                    "-noP4",
                    "-clientconfig=" + configuration,
                    "-serverconfig=" + configuration,
                    "-utf8output",
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

                bool isLinux = platform == "Linux";
                var assemblyPlatform = isLinux ? "Linux" : "Windows";
                var serverPath = Path.Combine(stagingDir, assemblyPlatform + "Server");

                if (isLinux)
                {
                    // Write out the wrapper shell script to work around issues between UnrealEngine and our cloud Linux environments.
                    // Also ensure script uses Linux line endings
                    File.WriteAllText(Path.Combine(serverPath, "StartWorker.sh"), string.Format(UnrealWorkerShellScript, baseGameName).Replace("\r\n", "\n"), new UTF8Encoding(false));
                }

                Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new[]
                {
                    "ZipUtils",
                    "-add=" + Quote(serverPath),
                    "-archive=" + Quote(Path.Combine(outputDir, $"UnrealWorker@{assemblyPlatform}.zip"))
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
