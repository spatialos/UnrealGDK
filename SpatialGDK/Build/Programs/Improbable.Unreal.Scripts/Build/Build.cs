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
LOG_FILE=$2
shift 2

# 2>/dev/null silences errors by redirecting stderr to the null device. This is done to prevent errors when a machine attempts to add the same user more than once.
useradd $NEW_USER -m -d /improbable/logs/UnrealWorker/Logs 2>/dev/null
chown -R $NEW_USER:$NEW_USER $(pwd) 2>/dev/null
chmod -R o+rw /improbable/logs 2>/dev/null

# Create log file in case it doesn't exist and redirect stdout and stderr to the file.
touch ""${{LOG_FILE}}""
exec 1>>""${{LOG_FILE}}""
exec 2>&1

SCRIPT=""$(pwd)/{0}Server.sh""

if [ ! -f $SCRIPT ]; then
    echo ""Expected to run ${{SCRIPT}} but file not found!""
    exit 1
fi

chmod +x $SCRIPT
echo ""Running ${{SCRIPT}} to start worker...""
gosu $NEW_USER ""${{SCRIPT}}"" ""$@""";


        // This is for internal use only. We do not support Linux clients.
        private const string SimulatedPlayerWorkerShellScript =
@"#!/bin/bash
NEW_USER=unrealworker
WORKER_ID=$1
WORKER_NAME=$2
shift 2

# 2>/dev/null silences errors by redirecting stderr to the null device. This is done to prevent errors when a machine attempts to add the same user more than once.
useradd $NEW_USER -m -d /improbable/logs/ >> ""/improbable/logs/${WORKER_ID}.log"" 2>&1
chown -R $NEW_USER:$NEW_USER $(pwd) >> ""/improbable/logs/${WORKER_ID}.log"" 2>&1
chmod -R o+rw /improbable/logs >> ""/improbable/logs/${WORKER_ID}.log"" 2>&1
SCRIPT=""$(pwd)/${WORKER_NAME}.sh""
chmod +x $SCRIPT >> ""/improbable/logs/${WORKER_ID}.log"" 2>&1

echo ""Trying to launch worker ${WORKER_NAME} with id ${WORKER_ID}"" > ""/improbable/logs/${WORKER_ID}.log""
gosu $NEW_USER ""${SCRIPT}"" ""$@"" >> ""/improbable/logs/${WORKER_ID}.log"" 2>&1";

        private const string RunEditorScript =
            @"setlocal ENABLEDELAYEDEXPANSION
%UNREAL_HOME%\Engine\Binaries\Win64\UE4Editor.exe ""{0}"" %*
exit /b !ERRORLEVEL!
";

        public static void Main(string[] args)
        {
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
                Console.WriteLine("Usage: <GameName> <Platform> <Configuration> <game.uproject> [-nocompile] <Additional UAT args>");

                Environment.Exit(exitCode);
            }

            var gameName = args[0];
            var platform = args[1];
            var configuration = args[2];
            var projectFile = Path.GetFullPath(args[3]);
            var noCompile = args.Count(arg => arg.ToLowerInvariant() == "-nocompile") > 0;
            var additionalUATArgs = string.Join(" ", args.Skip(4).Where(arg => arg.ToLowerInvariant() != "-nocompile"));

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
                    additionalUATArgs
                });

                var windowsNoEditorPath = Path.Combine(stagingDir, "WindowsNoEditor");

                // Add a _ to the start of the exe name, to ensure it is the exe selected by the launcher.
                // TO-DO: Remove this once LAUNCH-341 has been completed, and the _ is no longer necessary.
                var oldExe = Path.Combine(windowsNoEditorPath, $"{gameName}.exe");
                var renamedExe = Path.Combine(windowsNoEditorPath, $"_{gameName}.exe");
                if (File.Exists(renamedExe))
                {
                    File.Delete(renamedExe);
                }
                if (File.Exists(oldExe))
                {
                    File.Move(oldExe, renamedExe);
                }
                else
                {
                    Console.WriteLine("Could not find the executable to rename.");
                }

                Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new[]
                {
                    "ZipUtils",
                    "-add=" + Quote(windowsNoEditorPath),
                    "-archive=" + Quote(Path.Combine(outputDir, "UnrealClient@Windows.zip")),
                });
            }
            else if (gameName == baseGameName + "FakeClient")
            {
                Common.WriteWarning("'FakeClient' has been renamed to 'SimulatedPlayer', please use this instead. It will create the same assembly under a different name: UnrealSimulatedPlayer@Linux.zip.");
            }
            else if (gameName == baseGameName + "SimulatedPlayer") // This is for internal use only. We do not support Linux clients.
            {
                Common.WriteHeading(" > Building simulated player.");
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
                    "-nullrhi",
                    additionalUATArgs
                });

                var linuxSimulatedPlayerPath = Path.Combine(stagingDir, "LinuxNoEditor");
                File.WriteAllText(Path.Combine(linuxSimulatedPlayerPath, "StartWorker.sh"), SimulatedPlayerWorkerShellScript.Replace("\r\n", "\n"), new UTF8Encoding(false));

                var workerCoordinatorPath = Path.GetFullPath(Path.Combine("../spatial", "build", "dependencies", "WorkerCoordinator"));
                if (Directory.Exists(workerCoordinatorPath))
                {
                    Common.RunRedirected("xcopy", new[]
                    {
                        "/I",
                        "/Y",
                        workerCoordinatorPath,
                        linuxSimulatedPlayerPath
                    });
                } else
                {
                    Console.WriteLine("worker coordinator path did not exist");
                }

                var archiveFileName = "UnrealSimulatedPlayer@Linux.zip";
                Common.RunRedirected(@"%UNREAL_HOME%\Engine\Build\BatchFiles\RunUAT.bat", new[]
                {
                    "ZipUtils",
                    "-add=" + Quote(linuxSimulatedPlayerPath),
                    "-archive=" + Quote(Path.Combine(outputDir, archiveFileName)),
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
                    "-server",
                    "-serverplatform=" + platform,
                    "-noclient",
                    additionalUATArgs
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
