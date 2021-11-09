// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.IO;
using System.Linq;
using Improbable.Unreal.Build.Common;

namespace Improbable.Unreal.Build.Util
{
    public static class WriteLinuxScript
    {
        public static void Main(string[] args)
        {
            var help = args.Count(arg => arg == "/?" || arg.ToLowerInvariant() == "--help") > 0;

            var exitCode = 0;
            if (args.Length < 2 && !help)
            {
                help = true;
                exitCode = 1;
            }

            if (help)
            {
                Console.WriteLine("Usage: <OutputFile> <GameName>");
                Environment.Exit(exitCode);
            }

            var outputFile = Path.GetFullPath(args[0]);
            var baseGameName = args[1];

            Improbable.Common.WriteHeading(" > Writing Linux worker start script.");

            // Write out the wrapper shell script to work around issues between UnrealEngine and our cloud Linux environments.
            // Also ensure script uses Linux line endings
            LinuxScripts.WriteWithLinuxLineEndings(LinuxScripts.GetUnrealWorkerShellScript(baseGameName), outputFile);
        }
    }
}
