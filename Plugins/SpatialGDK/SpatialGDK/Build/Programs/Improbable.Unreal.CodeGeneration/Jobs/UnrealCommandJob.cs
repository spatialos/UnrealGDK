using System.Collections.Generic;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration.Jobs
{
    public class UnrealCommandJob : CodegenJob
    {
        public UnrealCommandJob(UnrealCommandDetails commandDetails, string outputDirectory, IFileSystem fileSystem)
            : base(outputDirectory, fileSystem)
        {
            InputFiles = new List<string>() { commandDetails.UnderlyingPackageDetails.SourceSchema };

            var commandResponderHeaderPath = commandDetails.CapitalisedName + commandResponderHeaderSuffix;
            var commandResponderImplPath = commandDetails.CapitalisedName + commandResponderImplSuffix;

            OutputFiles = new List<string>()
            {
                commandResponderHeaderPath,
                commandResponderImplPath,
            };

            this.commandDetails = commandDetails;
        }

        protected override void RunImpl()
        {
            var responderHeaderGenerator = new UnrealCommandResponderHeaderGenerator(commandDetails);
            var commandResponderHeaderPath = commandDetails.CapitalisedName + commandResponderHeaderSuffix;
            Content.Add(commandResponderHeaderPath, responderHeaderGenerator.TransformText());

            var responderImplementationGenerator = new UnrealCommandResponderImplementationGenerator(commandDetails);
            var commandResponderImplPath = commandDetails.CapitalisedName + commandResponderImplSuffix;
            Content.Add(commandResponderImplPath, responderImplementationGenerator.TransformText());
        }

        private const string commandResponderHeaderSuffix = "CommandResponder.h";
        private const string commandResponderImplSuffix = "CommandResponder.cpp";

        private readonly UnrealCommandDetails commandDetails;
    }
}
