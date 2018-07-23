using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Improbable
{
    public static class Linter
    {
        public static void Main(string[] args)
        {
            var help = args.Count(arg => arg == "/?" || arg.ToLowerInvariant() == "--help") > 0;

            if (args.Length == 0 || args.Length < 2)
            {
                help = true;
                exitCode = 1;
                Console.Error.WriteLine("Command and paths must be specified");
                PrintHelp();
            }

Console.WriteLine("TEST:");
            var command = args[0];
            if ((command != "check" && command != "fix") || help)
            {
                Console.WriteLine("Usage: [Command] <paths>");
                Console.WriteLine("Available Commands:");
                Console.WriteLine("\tcheck : Checks the paths for lint.");
                Console.WriteLine("\tfix: fix lint in the specified paths.");

                Environment.Exit(exitCode);
            }
Console.WriteLine("TEST:");
            try
            {
                var paths = args.Skip(1).ToList();
                if(command == "check")
                {
                    CheckLint(paths);
                }
                else
                {
                    TestLint(paths);
                }
            }
            catch (System.Exception e)
            {
                Console.Error.WriteLine(@"{0} Exception caught.", e);
                exitCode = 1;
            }

            Environment.Exit(exitCode);
        }

        private static void PrintHelp()
        {
            Console.WriteLine("Usage: [Command] <paths>");
            Console.WriteLine("Available Commands:");
            Console.WriteLine("\tcheck : Checks the paths for lint.");
            Console.WriteLine("\tfix: fix lint in the specified paths.");

            Environment.Exit(exitCode);
        }

        private static void CheckLint(List<string> paths)
        {
	        var pathsWithLint = GetFailedValidationPaths(paths);

            foreach(var path in pathsWithLint)
            {
                Console.WriteLine(@"{0} - missing copyright header", path);
            }

            if(pathsWithLint.Count > 0)
            {
                Console.Error.WriteLine(@"{0} files failed linting", pathsWithLint.Count);
            }
        }

        private static void TestLint(List<string> paths)
        {
            var pathsWithLint = GetFailedValidationPaths(paths);

            foreach(var path in pathsWithLint)
            {
                var content = File.ReadAllText(path);
                content = "// Copyright (c) Improbable Worlds Ltd, All Rights Reserved\r\n\r\n" + content;

                File.WriteAllText(path, content);

                Console.WriteLine(@"Fixed {0}", path);
            }
        }

        private static List<string> GetFailedValidationPaths(List<string> paths)
        {
            var directories = paths.Select(path => new DirectoryInfo(path));

            List<string> filesWithLint = new List<string>();

            foreach(var directory in directories)
            {
                if(!IsExcluded(directory))
                {
                    var files = directory.EnumerateFiles("*.*", SearchOption.AllDirectories)
                                         .Where(s => s.FullName.EndsWith(".h") || s.FullName.EndsWith(".cpp")|| s.FullName.EndsWith(".Build.cs"));

                    foreach(var file in files)
                    {
                        if(NeedsChanges(file))
                        {
                            filesWithLint.Add(file.FullName);
                        }
                    }
                }
            }

            return filesWithLint;
        }

        private static bool IsExcluded(DirectoryInfo Directory)
        {
            var result = Common.RunRedirected("git", new[]
            {
                "check-ignore",
                Directory.FullName
            });

            return result == 0;
        }

        private static bool NeedsChanges(FileInfo fileInfo)
        {
            var content = File.ReadAllText(fileInfo.FullName);

            if(!content.StartsWith("// Copyright"))
            {
                Console.Error.WriteLine(@"{0} is missing the copyright header", fileInfo.FullName);
                return true;
            }

            return false;
        }

        private static int exitCode = 0;
    }
}
