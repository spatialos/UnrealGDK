using System.Collections.Generic;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.CodeGeneration.Model;
using Improbable.Unreal.CodeGeneration.SchemaProcessing;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration.Jobs
{
    public class JobGenerator
    {
        public JobGenerator(IFileSystem fileSystem)
        {
            this.fileSystem = fileSystem;
        }

        public ICollection<ICodegenJob> GenerateJobs(ICollection<SchemaFileRaw> schemaFiles, CodeGeneratorOptions options)
        {
            var jobs = new List<ICodegenJob>();

            jobs.AddRange(GenerateUnrealJobs(schemaFiles, options));
            return jobs;
        }

        private ICollection<ICodegenJob> GenerateUnrealJobs(ICollection<SchemaFileRaw> schemaFiles, CodeGeneratorOptions options)
        {
            var jobs = new List<ICodegenJob>();

            var schemaProcessor = new UnrealSchemaProcessor(schemaFiles);

            jobs.AddRange(GenerateUnrealEnumJobs(schemaProcessor.unrealEnumDetails, options.OutputDir));
            jobs.AddRange(GenerateUnrealTypeJobs(schemaProcessor.unrealTypeDetails, options.OutputDir));
            jobs.AddRange(GenerateUnrealComponentJobs(schemaProcessor.unrealComponentDetails, options.OutputDir));
            jobs.AddRange(GenerateUnrealCommandJobs(schemaProcessor.unrealCommandDetails, options.OutputDir));
            jobs.AddRange(GenerateUnrealCommanderJobs(schemaProcessor.unrealCommandDetails, schemaProcessor.allPackageIncludes, options.OutputDir));
            jobs.AddRange(GenerateUnrealEntityTemplateJobs(schemaProcessor.unrealComponentDetails, schemaProcessor.allPackageIncludes, options.OutputDir));
            jobs.AddRange(GenerateUnrealEntityPipelineJobs(schemaProcessor.unrealComponentDetails, options.OutputDir));
            jobs.AddRange(GenerateCommonHeaderJobs(schemaProcessor.unrealComponentDetails, options.OutputDir));
            jobs.AddRange(GenerateUnrealCallbackDispatcherJobs(schemaProcessor.unrealComponentDetails, schemaProcessor.allPackageIncludes, options.OutputDir));

            return jobs;
        }

        private List<ICodegenJob> GenerateUnrealEnumJobs(List<UnrealEnumDetails> unrealEnums, string outputDirectory)
        {
            List<ICodegenJob> jobs = new List<ICodegenJob>();

            foreach (var unrealEnum in unrealEnums)
            {
                var enumJob = new UnrealEnumJob(unrealEnum, outputDirectory, fileSystem);
                jobs.Add(enumJob);
            }

            return jobs;
        }

        private List<ICodegenJob> GenerateUnrealTypeJobs(List<UnrealTypeDetails> unrealTypes, string outputDirectory)
        {
            List<ICodegenJob> jobs = new List<ICodegenJob>();

            var generatedMaps = new HashSet<string>();
            var generatedOptions = new HashSet<string>();
            var generatedLists = new HashSet<string>();

            foreach (var unrealType in unrealTypes)
            {
                var job = new UnrealTypeJob(unrealType, generatedMaps, generatedOptions, generatedLists, outputDirectory, fileSystem);
                jobs.Add(job);
            }

            return jobs;
        }

        private List<ICodegenJob> GenerateUnrealComponentJobs(List<UnrealComponentDetails> unrealComponents, string outputDirectory)
        {
            List<ICodegenJob> jobs = new List<ICodegenJob>();

            foreach (var unrealComponent in unrealComponents)
            {
                var job = new UnrealComponentJob(unrealComponent, outputDirectory, fileSystem);
                jobs.Add(job);
            }

            return jobs;
        }

        private List<ICodegenJob> GenerateUnrealCommandJobs(List<UnrealCommandDetails> unrealCommands, string outputDirectory)
        {
            List<ICodegenJob> jobs = new List<ICodegenJob>();

            foreach (var unrealCommand in unrealCommands)
            {
                var job = new UnrealCommandJob(unrealCommand, outputDirectory, fileSystem);
                jobs.Add(job);
            }

            return jobs;
        }

        private List<ICodegenJob> GenerateUnrealCommanderJobs(List<UnrealCommandDetails> unrealCommands, HashSet<string> allPackageIncludes, string outputDirectory)
        {
            return new List<ICodegenJob>() { new UnrealCommanderJob(unrealCommands, allPackageIncludes, outputDirectory, fileSystem) };
        }

        private List<ICodegenJob> GenerateUnrealEntityTemplateJobs(List<UnrealComponentDetails> unrealComponents, HashSet<string> allPackageIncludes, string outputDirectory)
        {
            return new List<ICodegenJob>() { new UnrealEntityTemplateJob(unrealComponents, allPackageIncludes, outputDirectory, fileSystem) };
        }

        private List<ICodegenJob> GenerateUnrealEntityPipelineJobs(List<UnrealComponentDetails> unrealComponents, string outputDirectory)
        {
            return new List<ICodegenJob>() { new UnrealEntityPipelineJob(unrealComponents, outputDirectory, fileSystem) };
        }

        private List<ICodegenJob> GenerateCommonHeaderJobs(List<UnrealComponentDetails> unrealComponents, string outputDirectory)
        {
            return new List<ICodegenJob>() { new UnrealCommonHeaderJob(unrealComponents, outputDirectory, fileSystem) };
        }

        private List<ICodegenJob> GenerateUnrealCallbackDispatcherJobs(List<UnrealComponentDetails> unrealComponents, HashSet<string> allPackageIncludes, string outputDirectory)
        {
            return new List<ICodegenJob>() { new UnrealCallbackDispatcherJob(unrealComponents, allPackageIncludes, outputDirectory, fileSystem) };
        }

        private readonly IFileSystem fileSystem;
    }
}
