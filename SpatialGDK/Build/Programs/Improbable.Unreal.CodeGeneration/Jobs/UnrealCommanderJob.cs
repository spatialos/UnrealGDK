using System.Collections.Generic;
using System.Linq;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration.Jobs
{
    public class UnrealCommanderJob : CodegenJob
    {
        public UnrealCommanderJob(List<UnrealCommandDetails> commandDetailsList, HashSet<string> allPackageIncludes, string outputDirectory, IFileSystem fileSystem)
            : base(outputDirectory, fileSystem)
        {
            InputFiles = commandDetailsList.Select(commandDetail => commandDetail.UnderlyingPackageDetails.SourceSchema).ToList();

            OutputFiles = new List<string>()
            {
                commmanderHeaderFileName,
                commmanderImplFileName
            };

            this.commandDetailsList = commandDetailsList;
            this.allPackageIncludes = allPackageIncludes;
        }

        protected override void RunImpl()
        {
            var commanderHeaderGenerator = new UnrealCommanderHeaderGenerator(commandDetailsList, allPackageIncludes);
            Content.Add(commmanderHeaderFileName, commanderHeaderGenerator.TransformText());

            var commanderImplementationGenerator = new UnrealCommanderImplementationGenerator(commandDetailsList);
            Content.Add(commmanderImplFileName, commanderImplementationGenerator.TransformText());
        }

        private const string commmanderHeaderFileName = "Commander.h";
        private const string commmanderImplFileName = "Commander.cpp";
        private readonly List<UnrealCommandDetails> commandDetailsList;
        private readonly HashSet<string> allPackageIncludes;
    }
}
