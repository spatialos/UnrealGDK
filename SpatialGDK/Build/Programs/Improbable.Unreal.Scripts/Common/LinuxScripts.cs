// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System.IO;
using System.Text;

namespace Improbable.Unreal.Build.Common
{
    public static class LinuxScripts
    {
        private const string UnrealWorkerShellScript =
@"#!/bin/bash
NEW_USER=unrealworker
WORKER_ID=$1
LOG_FILE=$2
shift 2

# 2>/dev/null silences errors by redirecting stderr to the null device. This is done to prevent errors when a machine attempts to add the same user more than once.
mkdir -p /improbable/logs/UnrealWorker/
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
        public const string SimulatedPlayerWorkerShellScript =
@"#!/bin/bash
NEW_USER=unrealworker
WORKER_ID=$1
shift 1

# 2>/dev/null silences errors by redirecting stderr to the null device. This is done to prevent errors when a machine attempts to add the same user more than once.
useradd $NEW_USER -m -d /improbable/logs/ >> ""/improbable/logs/${{WORKER_ID}}.log"" 2>&1
chown -R $NEW_USER:$NEW_USER $(pwd) >> ""/improbable/logs/${{WORKER_ID}}.log"" 2>&1
chmod -R o+rw /improbable/logs >> ""/improbable/logs/${{WORKER_ID}}.log"" 2>&1
SCRIPT=""$(pwd)/{0}.sh""
chmod +x $SCRIPT >> ""/improbable/logs/${{WORKER_ID}}.log"" 2>&1

echo ""Trying to launch worker {0} with id ${{WORKER_ID}}"" > ""/improbable/logs/${{WORKER_ID}}.log""
gosu $NEW_USER ""${{SCRIPT}}"" ""$@"" >> ""/improbable/logs/${{WORKER_ID}}.log"" 2>&1";

        public const string SimulatedPlayerCoordinatorShellScript =
@"#!/bin/sh
sleep 5

chmod +x WorkerCoordinator.exe
chmod +x StartSimulatedClient.sh
chmod +x {0}.sh

mono WorkerCoordinator.exe $@ 2> /improbable/logs/CoordinatorErrors.log";

        // Returns a version of UnrealWorkerShellScript with baseGameName templated into the right places.
        // baseGameName should be the base name of your Unreal game.
        public static string GetUnrealWorkerShellScript(string baseGameName)
        {
            return string.Format(UnrealWorkerShellScript, baseGameName);
        }

        // Returns a version of SimulatedPlayerWorkerShellScript with baseGameName templated into the right places.
        // baseGameName should be the base name of your Unreal game.
        public static string GetSimulatedPlayerWorkerShellScript(string baseGameName)
        {
            return string.Format(SimulatedPlayerWorkerShellScript, baseGameName);
        }

        // Returns a version of SimulatedPlayerCoordinatorShellScript with baseGameName templated into the right places.
        // baseGameName should be the base name of your Unreal game.
        public static string GetSimulatedPlayerCoordinatorShellScript(string baseGameName)
        {
            return string.Format(SimulatedPlayerCoordinatorShellScript, baseGameName);
        }

        // Writes out fileContents to fileName, ensuring that the resulting file has Linux line endings.
        public static void WriteWithLinuxLineEndings(string fileContents, string fileName)
        {
            File.WriteAllText(fileName, fileContents.Replace("\r\n", "\n"), new UTF8Encoding(false));
        }
    }
}
