using System.Collections.Generic;
using System.Linq;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration.Jobs
{
    public class UnrealEntityPipelineJob : CodegenJob
    {
        public UnrealEntityPipelineJob(List<UnrealComponentDetails> unrealComponents, string outputDirectory, IFileSystem fileSystem)
            : base(outputDirectory, fileSystem)
        {
            InputFiles = unrealComponents.Select(component => component.UnderlyingPackageDetails.SourceSchema).ToList();
            OutputFiles = new List<string>()
            {
                entityPipelineHeaderFileName,
                entityPipelineImplFileName
            };

            this.unrealComponents = unrealComponents;
        }

        protected override void RunImpl()
        {
            var entityPipelineHeaderGenerator = new UnrealEntityPipelineHeaderGenerator(unrealComponents);
            Content.Add(entityPipelineHeaderFileName, entityPipelineHeaderGenerator.TransformText());

            var entityPipelineImplementationGenerator = new UnrealEntityPipelineImplementationGenerator(unrealComponents);
            Content.Add(entityPipelineImplFileName, entityPipelineImplementationGenerator.TransformText());
        }

        private const string entityPipelineHeaderFileName = "EntityPipeline.h";
        private const string entityPipelineImplFileName = "EntityPipeline.cpp";
        private readonly List<UnrealComponentDetails> unrealComponents;
    }
}
