using System.Collections.Generic;
using System.Linq;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration.Jobs
{
    public class UnrealCallbackDispatcherJob : CodegenJob
    {
        public UnrealCallbackDispatcherJob(List<UnrealComponentDetails> unrealComponents, HashSet<string> allPackageIncludes, string outputDirectory, IFileSystem fileSystem)
            : base(outputDirectory, fileSystem)
        {
            InputFiles = unrealComponents.Select(component => component.UnderlyingPackageDetails.SourceSchema).ToList();
            OutputFiles = new List<string>()
            {
                callbackDispatcherHeaderFileName,
                callbackDispatcherImplFileName
            };

            this.unrealComponents = unrealComponents;
            this.allPackageIncludes = allPackageIncludes;
        }

        protected override void RunImpl()
        {
            var callbackDispatcherHeaderGenerator = new UnrealCallbackDispatcherHeaderGenerator(unrealComponents, allPackageIncludes);
            Content.Add(callbackDispatcherHeaderFileName, callbackDispatcherHeaderGenerator.TransformText());

            var callbackDispatcherImplementationGenerator = new UnrealCallbackDispatcherImplementationGenerator(unrealComponents);
            Content.Add(callbackDispatcherImplFileName, callbackDispatcherImplementationGenerator.TransformText());
        }

        private const string callbackDispatcherHeaderFileName = "CallbackDispatcher.h";
        private const string callbackDispatcherImplFileName = "CallbackDispatcher.cpp";
        private readonly List<UnrealComponentDetails> unrealComponents;
        private readonly HashSet<string> allPackageIncludes;
    }
}
