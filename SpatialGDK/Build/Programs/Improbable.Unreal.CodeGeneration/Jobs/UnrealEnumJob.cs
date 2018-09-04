using System.Collections.Generic;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration.Jobs
{
    public class UnrealEnumJob : CodegenJob
    {
        public UnrealEnumJob(UnrealEnumDetails unrealEnum, string outputDirectory, IFileSystem fileSystem)
            : base(outputDirectory, fileSystem)
        {
            this.unrealEnum = unrealEnum;

            var enumHeaderFileName = unrealEnum.CapitalisedName + headerSuffix;

            InputFiles = new List<string>() { unrealEnum.UnderlyingPackageDetails.SourceSchema };
            OutputFiles = new List<string>() { enumHeaderFileName };
        }

        protected override void RunImpl()
        {
            var enumHeaderFileName = unrealEnum.CapitalisedName + headerSuffix;
            var headerGenerator = new UnrealEnumHeaderGenerator(unrealEnum);
            Content.Add(enumHeaderFileName, headerGenerator.TransformText());
        }

        private const string headerSuffix = ".h";
        private readonly UnrealEnumDetails unrealEnum;
    }
}
