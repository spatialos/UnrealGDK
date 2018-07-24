using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace Improbable
{
    public static class Linter
    {
        public static void Main(string[] args)
        {
            var help = args.Count(arg => arg == "/?" || arg.ToLowerInvariant() == "--help") > 0;
            var validCommand = args.Count(arg => arg.ToLowerInvariant() == checkCommand || arg.ToLowerInvariant() == fixCommand) == 1;
            int exitCode = 0;
                        
            if (help || args.Length < 2 || !validCommand)
            {
                PrintHelp();
                exitCode = 1;
                Environment.Exit(exitCode);
            }

            try
            {
                var command = args.First().ToLower();
                var paths = args.Skip(1).ToList();
                switch(command)
                {
                    case checkCommand:
                        {
                            CheckLint(paths);
                        }
                        break;
                    case "fix":
                        {
                            FixLint(paths);
                        }
                        break;
                    default:
                        {
                            PrintHelp();
                            exitCode = 1;
                        }
                        break;
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
        }

        private static void CheckLint(List<string> paths)
        {
	        var pathsWithLint = GetFailedValidationPaths(paths);

            if(pathsWithLint.Count > 0)
            {
                Console.Error.WriteLine(@"{0} files failed linting", pathsWithLint.Count);
            }
        }

        private static void FixLint(List<string> paths)
        {
            var pathsWithLint = GetFailedValidationPaths(paths);

            foreach(var path in pathsWithLint)
            {
                var content = File.ReadAllText(path);
                content = copyrightHeader + content;
                File.WriteAllText(path, content, new UTF8Encoding(false));

                Console.WriteLine(@"Fixed {0}", path);
            }
        }

        private static List<string> GetFailedValidationPaths(List<string> paths)
        {
            var directories = paths.Select(path => new DirectoryInfo(path));

            List<string> filesWithLint = new List<string>();

            foreach(var directory in directories.Where(IsIncluded))
            {
                var files = directory.EnumerateFiles("*.*", SearchOption.AllDirectories)
                                     .Where(s => s.FullName.EndsWith(".h") || s.FullName.EndsWith(".cpp") || s.FullName.EndsWith(".Build.cs"));

                foreach (var file in files.Where(NeedsChanges))
                {
                    filesWithLint.Add(file.FullName);
                }
            }

            return filesWithLint;
        }

        private static bool IsIncluded(DirectoryInfo Directory)
        {
            var result = Common.RunRedirectedWithExitCode("git", new[]
            {
                "check-ignore",
                Directory.FullName
            });

            return result != 0;
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

        private const string copyrightHeader = "// Copyright (c) Improbable Worlds Ltd, All Rights Reserved\r\n\r\n";
        private const string checkCommand = "check";
        private const string fixCommand = "fix";
    }
}
