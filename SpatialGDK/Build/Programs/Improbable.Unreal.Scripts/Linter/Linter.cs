// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace Improbable
{
    public static class Linter
    {
        /// <summary>
        /// The linter currently only checks for missing copyright headers
        /// </summary>
        public static void Main(string[] args)
        {
            bool help = args.Count(arg => arg == "/?" || arg.ToLowerInvariant() == "--help") > 0;
            bool validCommand = args.Count(arg => arg.ToLowerInvariant() == checkCommand || arg.ToLowerInvariant() == fixCommand) == 1;
            int exitCode = 0;

            if (help || args.Length < 2 || !validCommand)
            {
                PrintHelp();
                Environment.Exit(exitCode);
            }

            try
            {
                string command = args.First().ToLower();
                List<string> paths = args.Skip(1).ToList();
                switch (command)
                {
                    case checkCommand:
                        {
                            CheckLint(paths);
                        }
                        break;
                    case fixCommand:
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
            Console.WriteLine("\tix: Fix lint in the specified paths.");
        }

        private static void CheckLint(List<string> paths)
        {
            List<string> pathsWithLint = GetFailedValidationPaths(paths);

            if (pathsWithLint.Count > 0)
            {
                Console.Error.WriteLine(@"{0} files failed linting", pathsWithLint.Count);
            }
        }

        private static void FixLint(List<string> paths)
        {
            List<string> pathsWithLint = GetFailedValidationPaths(paths);

            foreach (string path in pathsWithLint)
            {
                string content = File.ReadAllText(path);
                content = copyrightHeader + content;
                File.WriteAllText(path, content, new UTF8Encoding(false));

                Console.WriteLine(@"Fixed {0}", path);
            }
        }

        private static List<string> GetFailedValidationPaths(List<string> paths)
        {
            IEnumerable<DirectoryInfo> directories = paths.Select(path => new DirectoryInfo(path));

            List<string> filesWithLint = new List<string>();

            foreach (DirectoryInfo directory in directories.Where(IsIncluded))
            {
                IEnumerable<FileInfo> files = directory.EnumerateFiles("*.*", SearchOption.AllDirectories)
                                     .Where(s => s.FullName.EndsWith(".h") || s.FullName.EndsWith(".cpp") || s.FullName.EndsWith(".Build.cs"));

                foreach (FileInfo file in files.Where(NeedsChanges))
                {
                    filesWithLint.Add(file.FullName);
                }
            }

            return filesWithLint;
        }

        private static bool IsIncluded(DirectoryInfo Directory)
        {
            int result = Common.RunRedirectedWithExitCode("git", new[]
            {
                "check-ignore",
                Directory.FullName
            });

            return result != 0;
        }

        private static bool NeedsChanges(FileInfo fileInfo)
        {
            string content = File.ReadAllText(fileInfo.FullName);
            if (!content.StartsWith("// Copyright"))
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
