using System;
using System.IO;
using CommandLine;

namespace ReleaseTool
{
    public class BuildkiteMetadataSink : IDisposable
    {
        public interface IBuildkiteOptions
        {
            [Option("buildkite-metadata-path", HelpText = "The location of a file which we can write metadata key-pairs to.")]
            string MetadataFilePath { get; set; }
        }

        public static bool CanWrite(IBuildkiteOptions options)
        {
            return !string.IsNullOrEmpty(options.MetadataFilePath);
        }

        private StreamWriter outputFile;

        public BuildkiteMetadataSink(IBuildkiteOptions options)
        {
            if (!CanWrite(options))
            {
                throw new ArgumentException($"Cannot create a {nameof(BuildkiteMetadataSink)}. Metadata file path is null or empty.");
            }

            outputFile = File.AppendText(options.MetadataFilePath);
        }

        public void WriteMetadata(string key, string value)
        {
            outputFile.WriteLine($"{key},{value}");
        }

        public void Dispose()
        {
            outputFile.Flush();
            outputFile.Close();
            outputFile.Dispose();
            outputFile = null;
        }
    }
}
