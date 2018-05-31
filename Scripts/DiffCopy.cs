using System;
using System.IO;
using System.Linq;
using System.Security.Cryptography;

namespace Improbable
{
    public static class DiffCopy
    {
        public static void Main(string[] args)
        {
            var diffOnly = args.Count(arg => arg.ToLowerInvariant() == "--diff-only") == 1;
            var verbose = args.Count(arg => arg.ToLowerInvariant() == "--verbose") == 1 || diffOnly == true;
            var help = args.Count(arg => arg == "/?" || arg.ToLowerInvariant() == "--help") > 0;

            var exitCode = 0;
            if (args.Length < 2 && !help)
            {
                help = true;
                exitCode = 1;
                Console.Error.WriteLine("Input and outpu path must be specified.");
            }

            if (help)
            {
                Console.WriteLine("Usage: <InputFolder> <OutputFolder> [flags]");
                Console.WriteLine("Flags:");
                Console.WriteLine("\t--diff-only : only perform the diffing.");
                Console.WriteLine("\t--verbose : print the log to stdout.");

                Environment.Exit(exitCode);
            }

            var inputPath = Path.GetFullPath(args[0]);
            var outputPath = Path.GetFullPath(args[1]);

            //turn these into relative paths.
            var inputFiles = Directory.GetFiles(inputPath, "*.*", SearchOption.AllDirectories).Select(filePath => GetRelativePath(filePath, inputPath));
            var outputFiles = Directory.GetFiles(outputPath, "*.*", SearchOption.AllDirectories).Select(filePath => GetRelativePath(filePath, outputPath));

            //Ensure output files are up-to-date.
            foreach(var outputFile in outputFiles)
            {
                var inputFilePath = Path.Combine(inputPath, outputFile);
                var outputFilePath = Path.Combine(outputPath, outputFile);

                if(inputFiles.Contains(outputFile))
                {
                    var inputFileHash = CalculateMD5(inputFilePath);
                    var outputFileHash = CalculateMD5(outputFilePath);

                    if(inputFileHash != outputFileHash)
                    {
                        Log(verbose, diffOnly, string.Format("{0} is out of date, replacing with {1}", outputFilePath, inputFilePath));
                        if(!diffOnly)
                        {
                            File.Copy(inputFilePath, outputFilePath, true);
                        }
                    }
                    else
                    {
                        Log(verbose, diffOnly, string.Format("{0} is up-to-date", outputFilePath));
                    }
                }
                else
                {
                    if(!diffOnly)
                    {
                        Log(verbose, diffOnly, string.Format("{0} is stale, deleting", outputFilePath));
                        File.Delete(outputFilePath);
                    }
                }
            }

            //Copy over any files that are new.
            var newFiles = inputFiles.Except(outputFiles);
            foreach(var file in newFiles)
            {
                var inputFilePath = Path.Combine(inputPath, file);
                var outputFilePath = Path.Combine(outputPath, file);
                Log(verbose, diffOnly, string.Format("Copying new file {0} to {1}", inputFilePath, outputFilePath));
                File.Copy(inputFilePath, outputFilePath);
            }
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

        private static void Log(bool verbosityEnabled, bool diffOnly, string message)
        {
            var logMessage = message;
            if(diffOnly)
            {
                logMessage = string.Format("Diff-only mode enabled: {0}", message);
            }
            if(verbosityEnabled)
            {
                Console.WriteLine(logMessage);
            }
        }
    }
}
