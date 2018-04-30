using System.Collections.Generic;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;

namespace Improbable.Unreal.CodeGeneration.Test.Jobs
{
    internal class StubCodegenJob : CodegenJob
    {
        public StubCodegenJob(string outputDirectory, IFileSystem fileSystem, ICollection<string> inputFiles, ICollection<string> outputFiles)
            : base(outputDirectory, fileSystem)
        {
            InputFiles = inputFiles;
            OutputFiles = outputFiles;

            foreach (var entry in OutputFiles)
            {
                Content.Add(entry, string.Empty);
            }
        }

        protected override void RunImpl() { }
    }
}
