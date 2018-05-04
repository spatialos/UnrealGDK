using System.Collections.Generic;
using System.Linq;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration.Jobs
{
    public class UnrealCommonHeaderJob : CodegenJob
    {
        public UnrealCommonHeaderJob(List<UnrealComponentDetails> unrealComponents, string outputDirectory, IFileSystem fileSystem)
            : base(outputDirectory, fileSystem)
        {
            InputFiles = unrealComponents.Select(component => component.UnderlyingPackageDetails.SourceSchema).ToList();
            OutputFiles = new List<string>()
            {
                commonHeaderName
            };

            this.unrealComponents = unrealComponents;
        }

        protected override void RunImpl()
        {
            var commonHeaderGenerator = new UnrealCommonHeaderGenerator(unrealComponents);
            Content.Add(commonHeaderName, commonHeaderGenerator.TransformText());
        }

        private const string commonHeaderName = "SpatialOSCommon.h";
        private readonly List<UnrealComponentDetails> unrealComponents;
    }
}
