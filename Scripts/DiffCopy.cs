using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;

namespace Improbable
{
    public static class DiffCopy
    {
        public static void Main(string[] args)
        {
            diffOnly = args.Count(arg => arg.ToLowerInvariant() == "--diff-only") == 1;
            verbose = args.Count(arg => arg.ToLowerInvariant() == "--verbose") == 1 || diffOnly == true;
            var help = args.Count(arg => arg == "/?" || arg.ToLowerInvariant() == "--help") > 0;

            var exitCode = 0;
            if (args.Length < 2 && !help)
            {
                help = true;
                exitCode = 1;
                Console.Error.WriteLine("Input and output path must be specified.");
            }

            if (help)
            {
                Console.WriteLine("Usage: <InputFolder> <OutputFolder> [flags]");
                Console.WriteLine("Flags:");
                Console.WriteLine("\t--diff-only : only perform the diffing.");
                Console.WriteLine("\t--verbose : print the log to stdout.");

                Environment.Exit(exitCode);
            }

            try
            {
                var inputPath = new DirectoryInfo(args[0] + "/");
                var outputPath = new DirectoryInfo(args[1] + "/");

                if(outputPath.Parent == null)
                {
                    exitCode = 1;
                    Log(@"Output path is ROOT.");
                }

                // Turn these into relative paths.
                var inputFiles = new HashSet<string>(inputPath.GetFiles("*.*", SearchOption.AllDirectories).Select(fileInfo => GetRelativePath(fileInfo.FullName, inputPath.FullName)));
                var outputFiles = new HashSet<string>(outputPath.GetFiles("*.*", SearchOption.AllDirectories).Select(fileInfo => GetRelativePath(fileInfo.FullName, outputPath.FullName)));

                // Ensure output files are up-to-date.
                foreach(var outputFile in outputFiles)
                {
                    var inputFilePath = Path.Combine(inputPath.FullName, outputFile);
                    var outputFilePath = Path.Combine(outputPath.FullName, outputFile);

                    if(inputFiles.Contains(outputFile))
                    {
                        var inputFileHash = CalculateMD5(inputFilePath);
                        var outputFileHash = CalculateMD5(outputFilePath);

                        if(inputFileHash != outputFileHash)
                        {
                            Log(@"{0} is out of date, replacing with {1}", outputFilePath, inputFilePath);
                            CopyFile(inputFilePath, outputFilePath);
                        }
                        else
                        {
                            Log(@"{0} is up-to-date", outputFilePath);
                        }
                    }
                    else
                    {
                        Log(@"{0} is stale, deleting", outputFilePath);
                        DeleteFile(outputFilePath);
                    }
                }

                //Copy over any files that are new.
                var newFiles = inputFiles.Except(outputFiles);
                foreach(var file in newFiles)
                {
                    var inputFilePath = Path.Combine(inputPath.FullName, file);
                    var outputFilePath = Path.Combine(outputPath.FullName, file);
                    Log(@"Copying new file {0} to {1}", inputFilePath, outputFilePath);
                    CopyFile(inputFilePath, outputFilePath);
                }
            }
            catch (System.Exception e)
            {
                Log(@"{0} Exception caught.", e);
                exitCode = 1;
            }

            Environment.Exit(exitCode);
        }

        private static string GetRelativePath(string filePath, string folderPath)
        {
            Uri pathUri = new Uri(filePath);

            if (!folderPath.EndsWith(Path.DirectorySeparatorChar.ToString()))
            {
                folderPath += Path.DirectorySeparatorChar;
            }
            Uri folderUri = new Uri(folderPath);
            return Uri.UnescapeDataString(folderUri.MakeRelativeUri(pathUri).ToString().Replace('/', Path.DirectorySeparatorChar));
        }

        private static string CalculateMD5(string filename)
        {
            using (var md5 = MD5.Create())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = md5.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToLowerInvariant();
                }
            }
        }

        private static void Log(string format, params object[] args)
        {
            if(verbose)
            {
                var logMessage = string.Format(format, args);
                if(diffOnly)
                {
                    logMessage = $"Diff-only mode enabled: {logMessage}";
                }
                Console.WriteLine(logMessage);
            }
        }

        private static void CopyFile(string inputFilePath, string outputFilePath)
        {
            if(!diffOnly)
            {
                string directoryPath = Path.GetDirectoryName(outputFilePath);
                if (!Directory.Exists(directoryPath))
                {
                    Directory.CreateDirectory(directoryPath);
                }
                File.Copy(inputFilePath, outputFilePath, true);
            }
        }

        private static void DeleteFile(string file)
        {
            if(!diffOnly)
            {
                File.Delete(file);
            }
        }

        private static bool diffOnly;
        private static bool verbose;
    }
}
