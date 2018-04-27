using System.Collections.Generic;
using System.IO;
using System.Linq;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Test;
using Improbable.Unreal.CodeGeneration.Jobs;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.Jobs
{
    public class JobGeneratorTest
    {
        private static readonly CodeGeneratorOptions Options = CodeGeneratorOptions.ParseArguments(new[] { "--output-dir=UnrealTests" });

        [Test]
        public static void a_schema_with_an_enum_generates_the_expected_files_for_unreal()
        {
            var schemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.enum.json");
            var schemaFilesRaw = new List<SchemaFileRaw>() { schemaFileRaw };

            var jobGenerator = new JobGenerator(null);
            var jobs = jobGenerator.GenerateJobs(schemaFilesRaw, Options);

            Assert.That(jobs.Count == 8, "The wrong number of jobs were generated when a schema with an enum was processed");
            Assert.That(jobs.OfType<UnrealEnumJob>().Count() == 1, "Incorrect number of enum jobs were generated for the schema");

            var enumJob = jobs.OfType<UnrealEnumJob>().First();
            Assert.That(enumJob.InputFiles.Contains(EnumSchemaPath), "The input file for the enum job did not match the schema path");
            Assert.That(enumJob.OutputFiles.Count == 1, "The enum job has more output files than expected");
            Assert.That(enumJob.OutputFiles.First() == "ExampleColour.h", "The output path did not contain the expected file");

            Assert.That(jobs.OfType<UnrealComponentJob>().Count() == 1, "Incorrect number of component jobs were generated");
            Assert.That(jobs.OfType<UnrealTypeJob>().Count() == 1, "Incorrect number of type jobs were generated");
            Assert.That(jobs.OfType<UnrealCommanderJob>().Count() == 1, "Incorrect number of commander jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityTemplateJob>().Count() == 1, "Incorrect number of entity template jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityPipelineJob>().Count() == 1, "Incorrect number of entity pipeline jobs were generated");
            Assert.That(jobs.OfType<UnrealCommonHeaderJob>().Count() == 1, "Incorrect number of common header jobs were generated");
            Assert.That(jobs.OfType<UnrealCallbackDispatcherJob>().Count() == 1, "Incorrect number of callback dispatcher jobs were generated");

            var componentJob = jobs.OfType<UnrealComponentJob>().First();

            Assert.That(componentJob.InputFiles.Contains(EnumSchemaPath), string.Format("The input files {0} for the component job did not match that of the schema", string.Join(", ", componentJob.InputFiles.ToArray())));
            Assert.That(componentJob.OutputFiles.Count == 5, "Incorrect number of output files were created for the component job");

            Assert.That(componentJob.OutputFiles.Contains("ExampleEnumComponentComponent.h"), "The output files for the component job did not contain the expected file `ExampleEnumComponentComponent.h`");
            Assert.That(componentJob.OutputFiles.Contains("ExampleEnumComponentComponent.cpp"), "The output files for the component job did not contain the expected file `ExampleEnumComponentComponent.cpp`");
            Assert.That(componentJob.OutputFiles.Contains("ExampleEnumComponentComponentUpdate.h"), "The output files for the component job did not contain the expected file `ExampleEnumComponentComponentUpdate.h`");
            Assert.That(componentJob.OutputFiles.Contains("ExampleEnumComponentComponentUpdate.cpp"), "The output files for the component job did not contain the expected file `ExampleEnumComponentComponentUpdate.cpp`");
            Assert.That(componentJob.OutputFiles.Contains("ExampleEnumComponentAddComponentOp.h"), "The output files for the component job did not contain the expected file `ExampleEnumComponentAddComponentOp.h`");

            var typeJob = jobs.OfType<UnrealTypeJob>().First();

            Assert.That(typeJob.InputFiles.Contains(EnumSchemaPath), "The input file for the type job did not match that of the schema");
            Assert.That(typeJob.OutputFiles.Count == 2, "Incorrect number of output files were created for the type job");
            Assert.That(typeJob.OutputFiles.Contains("ExampleEnumComponentData.h"), "The output files for the type job did not contain the expected files");
            Assert.That(typeJob.OutputFiles.Contains("ExampleEnumComponentData.cpp"), "The output files for the type job did not contain the expected files");
        }

        [Test]
        public static void a_schema_with_a_command_generates_the_expected_files_for_unreal()
        {
            var jobGenerator = new JobGenerator(null);

            var schemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.command.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { schemaFileRaw };

            var jobs = jobGenerator.GenerateJobs(schemaFilesRaw, Options);

            Assert.That(jobs.Count == 10, "The wrong number of jobs were generated when a schema with a command was processed");

            Assert.That(jobs.Count(job => { return job.InputFiles.Count == 1; }) == 10, "Not all jobs had the expected number of input files");
            Assert.That(jobs.Count(job => { return job.InputFiles.Contains(ExampleComponentSchemaPath); }) == 10, "Not all jobs had the expected input file");


            Assert.That(jobs.OfType<UnrealComponentJob>().Count() == 1, "Incorrect number of component jobs were generated");
            Assert.That(jobs.OfType<UnrealTypeJob>().Count() == 3, "Incorrect number of type jobs were generated");
            Assert.That(jobs.OfType<UnrealCommandJob>().Count() == 1, "Incorrect number of type jobs were generated");
            Assert.That(jobs.OfType<UnrealCommanderJob>().Count() == 1, "Incorrect number of commander jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityTemplateJob>().Count() == 1, "Incorrect number of entity template jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityPipelineJob>().Count() == 1, "Incorrect number of entity pipeline jobs were generated");
            Assert.That(jobs.OfType<UnrealCommonHeaderJob>().Count() == 1, "Incorrect number of common header jobs were generated");
            Assert.That(jobs.OfType<UnrealCallbackDispatcherJob>().Count() == 1, "Incorrect number of callback dispatcher jobs were generated");

            var typeJobs = jobs.OfType<UnrealTypeJob>();

            Assert.That(typeJobs.FirstOrDefault((job) =>
            {
                var outputFiles = job.OutputFiles;
                return (outputFiles.Contains("ExampleRequest.h") &&
                        outputFiles.Contains("ExampleRequest.cpp") &&
                        outputFiles.Count == 2);
            }) != null, "The generated type jobs for a schema with a command did not produce the expected command request type output files");

            Assert.That(typeJobs.FirstOrDefault((job) =>
            {
                var outputFiles = job.OutputFiles;
                return (outputFiles.Contains("ExampleResponse.h") &&
                        outputFiles.Contains("ExampleResponse.cpp") &&
                        outputFiles.Count == 2);
            }) != null, "The generated type jobs for a schema with a command did not produce the expected command response type output files");

            Assert.That(typeJobs.FirstOrDefault((job) =>
            {
                var outputFiles = job.OutputFiles;
                return (outputFiles.Contains("ExampleComponentData.h") &&
                        outputFiles.Contains("ExampleComponentData.cpp") &&
                        outputFiles.Count == 2);
            }) != null, "The generated type jobs for a schema with a command did not produce the expected component type output files");

            var componentJobs = jobs.OfType<UnrealComponentJob>();

            Assert.That(componentJobs.FirstOrDefault((job) =>
            {
                var outputFiles = job.OutputFiles;
                return (outputFiles.Contains("ExampleComponentComponent.h") &&
                        outputFiles.Contains("ExampleComponentComponent.cpp") &&
                        outputFiles.Contains("ExampleComponentComponentUpdate.h") &&
                        outputFiles.Contains("ExampleComponentComponentUpdate.cpp") &&
                        outputFiles.Contains("ExampleComponentAddComponentOp.h") &&
                        outputFiles.Count == 5);
            }) != null, "The generated type jobs for a schema with a command did not produce the expected component output files");

            var commandJobs = jobs.OfType<UnrealCommandJob>();

            Assert.That(commandJobs.FirstOrDefault((job) =>
            {
                var outputFiles = job.OutputFiles;
                return (outputFiles.Contains("ExampleCommandCommandResponder.h") &&
                        outputFiles.Contains("ExampleCommandCommandResponder.cpp") &&
                        outputFiles.Count == 2);
            }) != null, "The generated type jobs for a schema with a command did not produce the expected command output files");
        }

        [Test]
        public static void a_schema_with_a_component_generates_the_expected_files_for_unreal()
        {
            var jobGenerator = new JobGenerator(null);

            var schemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.empty_component.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { schemaFileRaw };

            var jobs = jobGenerator.GenerateJobs(schemaFilesRaw, Options);
            Assert.That(jobs.Count() == 7, "The wrong number of jobs were generated when a schema with an empty component was processed");

            Assert.That(jobs.OfType<UnrealComponentJob>().Count() == 1, "Incorrect number of component jobs were generated");
            Assert.That(jobs.OfType<UnrealTypeJob>().Count() == 1, "Incorrect number of type jobs were generated");
            Assert.That(jobs.OfType<UnrealCommanderJob>().Count() == 1, "Incorrect number of commander jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityTemplateJob>().Count() == 1, "Incorrect number of entity template jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityPipelineJob>().Count() == 1, "Incorrect number of entity pipeline jobs were generated");
            Assert.That(jobs.OfType<UnrealCommonHeaderJob>().Count() == 1, "Incorrect number of common header jobs were generated");
            Assert.That(jobs.OfType<UnrealCallbackDispatcherJob>().Count() == 1, "Incorrect number of callback dispatcher jobs were generated");

            var componentJob = jobs.OfType<UnrealComponentJob>().First();

            Assert.That(componentJob.InputFiles.Contains(EmptyComponentSchemaPath), "The input file for the component job did not match that of the schema");
            Assert.That(componentJob.OutputFiles.Count == 5, "Incorrect number of output files were created for the component job");

            Assert.That(componentJob.OutputFiles.Contains("EmptyComponentComponent.h"), "The output files for the component job did not contain the expected file `EmptyComponentComponent.h`");
            Assert.That(componentJob.OutputFiles.Contains("EmptyComponentComponent.cpp"), "The output files for the component job did not contain the expected file `EmptyComponentComponent.cpp`");
            Assert.That(componentJob.OutputFiles.Contains("EmptyComponentComponentUpdate.h"), "The output files for the component job did not contain the expected file `EmptyComponentComponentUpdate.h`");
            Assert.That(componentJob.OutputFiles.Contains("EmptyComponentComponentUpdate.cpp"), "The output files for the component job did not contain the expected file `EmptyComponentComponentUpdate.cpp`");
            Assert.That(componentJob.OutputFiles.Contains("EmptyComponentAddComponentOp.h"), "The output files for the component job did not contain the expected file `EmptyComponentAddComponentOp.h`");

            var typeJob = jobs.OfType<UnrealTypeJob>().First();

            Assert.That(typeJob.InputFiles.Contains(EmptyComponentSchemaPath), "The input file for the type job did not match that of the schema");
            Assert.That(typeJob.OutputFiles.Count == 2, "Incorrect number of output files were created for the type job");
            Assert.That(typeJob.OutputFiles.Contains("EmptyComponentData.h"), "The output files for the type job did not contain the expected files");
            Assert.That(typeJob.OutputFiles.Contains("EmptyComponentData.cpp"), "The output files for the type job did not contain the expected files");
        }

        [Test]
        public static void two_schemas_with_a_command_each_generates_a_commander_with_expected_inputs_and_outputs_for_unreal()
        {
            var jobGenerator = new JobGenerator(null);

            var firstSchemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.command.json");
            var secondSchemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.commander.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { firstSchemaFileRaw, secondSchemaFileRaw };
            var jobs = jobGenerator.GenerateJobs(schemaFilesRaw, Options);

            Assert.That(jobs.Count == 15, "The wrong number of jobs were generated when two schemas with a commands were processed");

            Assert.That(jobs.OfType<UnrealComponentJob>().Count() == 2, "Incorrect number of component jobs were generated");
            Assert.That(jobs.OfType<UnrealTypeJob>().Count() == 6, "Incorrect number of type jobs were generated");
            Assert.That(jobs.OfType<UnrealCommandJob>().Count() == 2, "Incorrect number of command jobs were generated");
            Assert.That(jobs.OfType<UnrealCommanderJob>().Count() == 1, "Incorrect number of commander jobs were generated");
            Assert.That(jobs.OfType<UnrealCommonHeaderJob>().Count() == 1, "Incorrect number of common header jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityTemplateJob>().Count() == 1, "Incorrect number of entity template jobs were generated");

            var commanderJobs = jobs.OfType<UnrealCommanderJob>();

            Assert.That(commanderJobs.FirstOrDefault((job) =>
            {
                var inputFiles = job.InputFiles;
                return (inputFiles.Contains(ExampleComponentSchemaPath) &&
                        inputFiles.Contains(CommanderSchemaPath) &&
                        inputFiles.Count == 2);
            }) != null, "The generated commander did not have the expected input files");

            Assert.That(commanderJobs.FirstOrDefault((job) =>
            {
                var outputFiles = job.OutputFiles;
                return (outputFiles.Contains("Commander.h") &&
                        outputFiles.Contains("Commander.cpp") &&
                        outputFiles.Count == 2);
            }) != null, "The generated commander did not have the expected output files");
        }

        [Test]
        public static void two_schemas_with_a_commands_each_generates_an_entity_pipeline_with_expected_inputs_and_outputs_for_unreal()
        {
            var jobGenerator = new JobGenerator(null);

            var firstSchemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.command.json");
            var secondSchemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.commander.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { firstSchemaFileRaw, secondSchemaFileRaw };
            var jobs = jobGenerator.GenerateJobs(schemaFilesRaw, Options);

            Assert.That(jobs.Count == 15, "The wrong number of jobs were generated when two schemas with a commands were processed");

            Assert.That(jobs.OfType<UnrealComponentJob>().Count() == 2, "Incorrect number of component jobs were generated");
            Assert.That(jobs.OfType<UnrealTypeJob>().Count() == 6, "Incorrect number of type jobs were generated");
            Assert.That(jobs.OfType<UnrealCommandJob>().Count() == 2, "Incorrect number of command jobs were generated");
            Assert.That(jobs.OfType<UnrealCommanderJob>().Count() == 1, "Incorrect number of commander jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityTemplateJob>().Count() == 1, "Incorrect number of entity template jobs were generated");
            Assert.That(jobs.OfType<UnrealCommonHeaderJob>().Count() == 1, "Incorrect number of entity template jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityPipelineJob>().Count() == 1, "Incorrect number of entity pipeline jobs were generated");

            var entityPipelineJobs = jobs.OfType<UnrealEntityPipelineJob>();

            Assert.That(entityPipelineJobs.FirstOrDefault((job) =>
            {
                var inputFiles = job.InputFiles;
                return (inputFiles.Contains(ExampleComponentSchemaPath) &&
                        inputFiles.Contains(CommanderSchemaPath) &&
                        inputFiles.Count == 2);
            }) != null, "The generated commander did not have the expected input files");

            Assert.That(entityPipelineJobs.FirstOrDefault((job) =>
            {
                var outputFiles = job.OutputFiles;
                return (outputFiles.Contains("EntityPipeline.h") &&
                        outputFiles.Contains("EntityPipeline.cpp") &&
                        outputFiles.Count == 2);
            }) != null, "The generated commander did not have the expected output files");
        }

        [Test]
        public static void two_schemas_with_a_commands_each_generates_a_callback_dispatcher_with_expected_inputs_and_outputs_for_unreal()
        {
            var jobGenerator = new JobGenerator(null);

            var firstSchemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.command.json");
            var secondSchemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.commander.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { firstSchemaFileRaw, secondSchemaFileRaw };
            var jobs = jobGenerator.GenerateJobs(schemaFilesRaw, Options);

            Assert.That(jobs.Count == 15, "The wrong number of jobs were generated when two schemas with a commands were processed");

            Assert.That(jobs.OfType<UnrealComponentJob>().Count() == 2, "Incorrect number of component jobs were generated");
            Assert.That(jobs.OfType<UnrealTypeJob>().Count() == 6, "Incorrect number of type jobs were generated");
            Assert.That(jobs.OfType<UnrealCommandJob>().Count() == 2, "Incorrect number of command jobs were generated");
            Assert.That(jobs.OfType<UnrealCommanderJob>().Count() == 1, "Incorrect number of commander jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityTemplateJob>().Count() == 1, "Incorrect number of entity template jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityPipelineJob>().Count() == 1, "Incorrect number of entity pipeline jobs were generated");
            Assert.That(jobs.OfType<UnrealCallbackDispatcherJob>().Count() == 1, "Incorrect number of callback dispatcher jobs were generated");

            var callbackDispatcherJobs = jobs.OfType<UnrealCallbackDispatcherJob>();

            Assert.That(callbackDispatcherJobs.FirstOrDefault((job) =>
            {
                var inputFiles = job.InputFiles;
                return (inputFiles.Contains(ExampleComponentSchemaPath) &&
                        inputFiles.Contains(CommanderSchemaPath) &&
                        inputFiles.Count == 2);
            }) != null, "The generated commander did not have the expected input files");

            Assert.That(callbackDispatcherJobs.FirstOrDefault((job) =>
            {
                var outputFiles = job.OutputFiles;
                return (outputFiles.Contains("CallbackDispatcher.h") &&
                        outputFiles.Contains("CallbackDispatcher.cpp") &&
                        outputFiles.Count == 2);
            }) != null, "The generated commander did not have the expected output files");
        }

        [Test]
        public static void two_schemas_with_a_component_each_generates_an_entity_template_job_with_expected_inputs_and_outputs_for_unreal()
        {
            var jobGenerator = new JobGenerator(null);

            var firstSchemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.empty_component.json");
            var secondSchemaFileRaw = TestUtils.GetRawJsonDataFromResource<SchemaFileRaw>("Improbable.CodeGeneration.Resources.json.test.entity_template.json");

            var schemaFilesRaw = new List<SchemaFileRaw> { firstSchemaFileRaw, secondSchemaFileRaw };
            var jobs = jobGenerator.GenerateJobs(schemaFilesRaw, Options);

            Assert.That(jobs.Count == 9, "The wrong number of jobs were generated when two schemas with a component were processed");

            Assert.That(jobs.OfType<UnrealComponentJob>().Count() == 2, "Incorrect number of component jobs were generated");
            Assert.That(jobs.OfType<UnrealTypeJob>().Count() == 2, "Incorrect number of type jobs were generated");
            Assert.That(jobs.OfType<UnrealCommanderJob>().Count() == 1, "Incorrect number of commander jobs were generated");
            Assert.That(jobs.OfType<UnrealCommonHeaderJob>().Count() == 1, "Incorrect number of common header jobs were generated");
            Assert.That(jobs.OfType<UnrealEntityTemplateJob>().Count() == 1, "Incorrect number of entity template jobs were generated");

            var entityTemplateJob = jobs.OfType<UnrealEntityTemplateJob>();

            Assert.That(entityTemplateJob.FirstOrDefault((job) =>
            {
                var inputFiles = job.InputFiles;
                return (inputFiles.Contains(EmptyComponentSchemaPath) &&
                        inputFiles.Contains(EntityTemplateSchemaPath) &&
                        inputFiles.Count == 2);
            }) != null, "The generated commander did not have the expected input files");

            Assert.That(entityTemplateJob.FirstOrDefault((job) =>
            {
                var outputFiles = job.OutputFiles;
                return (outputFiles.Contains("EntityTemplate.h") &&
                        outputFiles.Contains("EntityTemplate.cpp") &&
                        outputFiles.Count == 2);
            }) != null, "The generated commander did not have the expected output files");
        }

        private static readonly string ExampleComponentSchemaPath = "Improbable.CodeGeneration/Resources/schema/command.schema";
        private static readonly string EnumSchemaPath = "Improbable.CodeGeneration/Resources/schema/enum.schema";
        private static readonly string EmptyComponentSchemaPath = "Improbable.CodeGeneration/Resources/schema/empty_component.schema";
        private static readonly string CommanderSchemaPath = "Improbable.CodeGeneration/Resources/schema/commander.schema";
        private static readonly string EntityTemplateSchemaPath = "Improbable.CodeGeneration/Resources/schema/entity_template.schema";
    }
}
