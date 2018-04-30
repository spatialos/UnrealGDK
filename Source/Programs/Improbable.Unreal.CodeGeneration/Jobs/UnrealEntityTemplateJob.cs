using System.Collections.Generic;
using System.Linq;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration.Jobs
{
    public class UnrealEntityTemplateJob : CodegenJob
    {
        public UnrealEntityTemplateJob(List<UnrealComponentDetails> unrealComponents, HashSet<string> allPackageIncludes, string outputDirectory, IFileSystem fileSystem)
            : base(outputDirectory, fileSystem)
        {
            InputFiles = unrealComponents.Select(component => component.UnderlyingPackageDetails.SourceSchema).ToList();
            OutputFiles = new List<string>();

            OutputFiles.Add(entityTemplateHeaderFileName);
            OutputFiles.Add(entityTemplateImplFileName);

            this.unrealComponents = unrealComponents;
            this.allPackageIncludes = allPackageIncludes;
        }

        protected override void RunImpl()
        {
            var entityTemplateHeaderGenerator = new UnrealEntityTemplateHeaderGenerator(unrealComponents, allPackageIncludes);
            Content.Add(entityTemplateHeaderFileName, entityTemplateHeaderGenerator.TransformText());

            var entityTemplateImplementationGenerator = new UnrealEntityTemplateImplementationGenerator(unrealComponents);
            Content.Add(entityTemplateImplFileName, entityTemplateImplementationGenerator.TransformText());
        }

        private const string entityTemplateHeaderFileName = "EntityTemplate.h";
        private const string entityTemplateImplFileName = "EntityTemplate.cpp";
        private readonly List<UnrealComponentDetails> unrealComponents;
        private readonly HashSet<string> allPackageIncludes;
    }
}
