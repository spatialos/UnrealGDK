using System;
using System.Collections.Generic;
using System.IO;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Test;
using Improbable.Unreal.CodeGeneration.Jobs;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.Jobs
{
    public class JobRunnerTest
    {
        private static readonly string OutputDirectory = Path.Combine("OutputDir", "test");
        private static readonly string TestSchemaPath = Path.Combine("improbable", Path.Combine("codegen", "Test.schema"));

        [Test]
        public static void run_executes_the_expected_file_operations_when_running_an_unrealcallbackdispatcherjob()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var file = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                file.ExistsMock = () => { return false; };
                return file;
            };
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpattern, recursive) => { return new List<IFile>(); };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", "improbable\\codegen\\Test.schema", "improbable.codegen");
            var unrealComponentDetails = new UnrealComponentDetails(GenerateComponentDefinition(), "TestComponent", null, null, null, unrealPackageDetails);
            var callbackDispatcherJob = new UnrealCallbackDispatcherJob(new List<UnrealComponentDetails> { unrealComponentDetails }, new HashSet<string>(), Path.Combine("OutputDir", "test"), mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { callbackDispatcherJob }, new List<string>() { OutputDirectory });

            Assert.That(mockFileSystem.DirectoryExistsCallCount == 2);
            Assert.That(mockFileSystem.WriteToFileCallCount == 2);
            Assert.That(mockFileSystem.WrittenFiles.Count == 2);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(Path.Combine("OutputDir", "test"), "CallbackDispatcher.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(Path.Combine("OutputDir", "test"), "CallbackDispatcher.cpp")));
        }

        [Test]
        public static void run_executes_the_expected_file_operations_when_running_an_unrealcommanderjob()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var file = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                file.ExistsMock = () => { return false; };
                return file;
            };
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpattern, recursive) => { return new List<IFile>(); };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", TestSchemaPath, "improbable.codegen");
            var responseTypeReference = new UnrealBuiltInTypeReference(GenerateTypeReferenceRaw());
            var requestTypeReference = new UnrealBuiltInTypeReference(GenerateTypeReferenceRaw());
            var unrealCommandDetails = new UnrealCommandDetails(GenerateCommandDefinition(), "TestCommand", "improbable.codegen.TestComponent", "TestComponent", requestTypeReference, responseTypeReference, unrealPackageDetails);
            var commanderJob = new UnrealCommanderJob(new List<UnrealCommandDetails> { unrealCommandDetails }, new HashSet<string>(), Path.Combine("OutputDir", "test"), mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { commanderJob }, new List<string>() { OutputDirectory });

            Assert.That(mockFileSystem.DirectoryExistsCallCount == 2);
            Assert.That(mockFileSystem.WriteToFileCallCount == 2);
            Assert.That(mockFileSystem.WrittenFiles.Count == 2);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "Commander.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "Commander.cpp")));
        }

        [Test]
        public static void run_executes_the_expected_file_operations_when_running_an_unrealcommandjob()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var file = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                file.ExistsMock = () => { return false; };
                return file;
            };
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpattern, recursive) => { return new List<IFile>(); };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", TestSchemaPath, "improbable.codegen");
            var responseTypeReference = new UnrealBuiltInTypeReference(GenerateTypeReferenceRaw());
            var requestTypeReference = new UnrealBuiltInTypeReference(GenerateTypeReferenceRaw());
            var unrealCommandDetails = new UnrealCommandDetails(GenerateCommandDefinition(), "TestCommand", "improbable.codegen.TestComponent", "TestComponent", requestTypeReference, responseTypeReference, unrealPackageDetails);
            var commandJob = new UnrealCommandJob(unrealCommandDetails, Path.Combine("OutputDir", "test"), mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { commandJob }, new List<string>() { OutputDirectory });

            Assert.That(mockFileSystem.DirectoryExistsCallCount == 2);
            Assert.That(mockFileSystem.WriteToFileCallCount == 2);
            Assert.That(mockFileSystem.WrittenFiles.Count == 2);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestCommandCommandResponder.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestCommandCommandResponder.cpp")));
        }

        [Test]
        public static void run_executes_the_expected_file_operations_when_running_an_unrealcomponentjob()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var file = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                file.ExistsMock = () => { return false; };
                return file;
            };
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpattern, recursive) => { return new List<IFile>(); };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", TestSchemaPath, "improbable.codegen");
            var unrealComponentDetails = new UnrealComponentDetails(GenerateComponentDefinition(), "TestComponent", new List<UnrealFieldDetails>(), new List<UnrealEventDetails>(), new List<UnrealCommandDetails>(), unrealPackageDetails);
            var componentJob = new UnrealComponentJob(unrealComponentDetails, Path.Combine("OutputDir", "test"), mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { componentJob }, new List<string>() { OutputDirectory });

            Assert.That(mockFileSystem.DirectoryExistsCallCount == 5);
            Assert.That(mockFileSystem.WriteToFileCallCount == 5);
            Assert.That(mockFileSystem.WrittenFiles.Count == 5);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestComponentComponent.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestComponentComponent.cpp")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestComponentComponentUpdate.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestComponentComponentUpdate.cpp")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestComponentAddComponentOp.h")));
        }

        [Test]
        public static void run_executes_the_expected_file_operations_when_running_an_unrealentitypipelinejob()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var file = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                file.ExistsMock = () => { return false; };
                return file;
            };
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpattern, recursive) => { return new List<IFile>(); };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", "improbable\\codegen\\Test.schema", "improbable.codegen");
            var unrealComponentDetails = new UnrealComponentDetails(GenerateComponentDefinition(), "TestComponent", new List<UnrealFieldDetails>(), new List<UnrealEventDetails>(), new List<UnrealCommandDetails>(), unrealPackageDetails);
            var entityPipelineJob = new UnrealEntityPipelineJob(new List<UnrealComponentDetails> { unrealComponentDetails }, Path.Combine("OutputDir", "test"), mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { entityPipelineJob }, new List<string>() { OutputDirectory });

            Assert.That(mockFileSystem.DirectoryExistsCallCount == 2);
            Assert.That(mockFileSystem.WriteToFileCallCount == 2);
            Assert.That(mockFileSystem.WrittenFiles.Count == 2);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(Path.Combine("OutputDir", "test"), "EntityPipeline.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(Path.Combine("OutputDir", "test"), "EntityPipeline.cpp")));
        }

        [Test]
        public static void run_executes_the_expected_file_operations_when_running_an_unrealentitytemplatejob()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var file = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                file.ExistsMock = () => { return false; };
                return file;
            };
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpattern, recursive) => { return new List<IFile>(); };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", TestSchemaPath, "improbable.codegen");
            var unrealComponentDetails = new UnrealComponentDetails(GenerateComponentDefinition(), "TestComponent", new List<UnrealFieldDetails>(), new List<UnrealEventDetails>(), new List<UnrealCommandDetails>(), unrealPackageDetails);
            var entityTemplateJob = new UnrealEntityTemplateJob(new List<UnrealComponentDetails> { unrealComponentDetails }, new HashSet<string>(), Path.Combine("OutputDir", "test"), mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { entityTemplateJob }, new List<string>() { OutputDirectory });

            Assert.That(mockFileSystem.DirectoryExistsCallCount == 2);
            Assert.That(mockFileSystem.WriteToFileCallCount == 2);
            Assert.That(mockFileSystem.WrittenFiles.Count == 2);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "EntityTemplate.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "EntityTemplate.cpp")));
        }

        [Test]
        public static void run_executes_the_expected_file_operations_when_running_an_unrealenumjob()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var file = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                file.ExistsMock = () => { return false; };
                return file;
            };
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpattern, recursive) => { return new List<IFile>(); };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", TestSchemaPath, "improbable.codegen");
            var unrealEnumDetails = new UnrealEnumDetails(GenerateEnumDefinition(), "TestEnum", unrealPackageDetails);
            var enumJob = new UnrealEnumJob(unrealEnumDetails, OutputDirectory, mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { enumJob }, new List<string>() { OutputDirectory });

            Assert.That(mockFileSystem.DirectoryExistsCallCount == 1);
            Assert.That(mockFileSystem.WriteToFileCallCount == 1);
            Assert.That(mockFileSystem.WrittenFiles.Count == 1);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestEnum.h")));
        }

        [Test]
        public static void run_executes_the_expected_file_operations_when_running_an_unrealtypejob()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var file = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                file.ExistsMock = () => { return false; };
                return file;
            };
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpattern, recursive) => { return new List<IFile>(); };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", TestSchemaPath, "improbable.codegen");
            var unrealTypeDetails = new UnrealTypeDetails(GenerateTypeDefinition("TestType", "improbable.codegen.TestType"), "TestType", new List<UnrealFieldDetails>(), unrealPackageDetails);
            var typeJob = new UnrealTypeJob(unrealTypeDetails, new HashSet<string>(), new HashSet<string>(), new HashSet<string>(), Path.Combine("OutputDir", "test"), mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { typeJob }, new List<string>() { OutputDirectory });

            Assert.That(mockFileSystem.DirectoryExistsCallCount == 2);
            Assert.That(mockFileSystem.WriteToFileCallCount == 2);
            Assert.That(mockFileSystem.WrittenFiles.Count == 2);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestType.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestType.cpp")));
        }

        [Test]
        public static void run_does_not_clean_output_folders_of_jobs_if_the_output_folder_contains_the_files_to_be_generated()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpatter, recursive) =>
            {
                return new List<IFile>()
                {
                    new MockFileWrapper(Path.Combine(path, "TestType.h"), path, new DateTime(1, 1, 1)),
                    new MockFileWrapper(Path.Combine(path, "TestType.cpp"), path, new DateTime(1, 1, 1))
                };
            };

            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var fileMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                fileMock.ExistsMock = () => { return true; };
                return fileMock;
            };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", TestSchemaPath, "improbable.codegen");
            var cleanUnrealTypeDetails = new UnrealTypeDetails(GenerateTypeDefinition("TestType", "improbable.codegen.TestType"), "TestType", new List<UnrealFieldDetails>(), unrealPackageDetails);
            var cleanTypeJob = new UnrealTypeJob(cleanUnrealTypeDetails, new HashSet<string>(), new HashSet<string>(), new HashSet<string>(), OutputDirectory, mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { cleanTypeJob }, new List<string>() { OutputDirectory });


            //This validates that the output folder was not deleted
            Assert.That(mockFileSystem.DeleteDirectoryCallCount == 0);
            Assert.That(mockFileSystem.GetFilesInDirectoryCallCount == 1);

            //This validates that no codegen jobs were run as they were expected to not be dirty.
            Assert.That(mockFileSystem.WriteToFileCallCount == 0);
            Assert.That(mockFileSystem.DirectoryExistsCallCount == 0);
            Assert.That(mockFileSystem.DirectoryExistsCallCount == 0);
            Assert.That(mockFileSystem.WriteToFileCallCount == 0);
            Assert.That(cleanTypeJob.IsDirty() == false);
        }

        [Test]
        public static void run_does_not_clean_if_new_files_are_required_but_all_previous_files_matches_those_of_the_expected_output()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpatter, recursive) =>
            {
                return new List<IFile>()
                {
                    new MockFileWrapper(Path.Combine(path, "TestType.h"), path, new DateTime(1, 1, 1)),
                    new MockFileWrapper(Path.Combine(path, "TestType.cpp"), path, new DateTime(1, 1, 1))
                };
            };

            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var fileMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));

                bool fileExists = false;
                var fileName = Path.GetFileName(path);
                if (fileName == "TestType.h" || fileName == "TestType.cpp")
                {
                    fileExists = true;
                }

                fileMock.ExistsMock = () => { return fileExists; };
                return fileMock;
            };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", TestSchemaPath, "improbable.codegen");

            var cleanUnrealTypeDetails = new UnrealTypeDetails(GenerateTypeDefinition("TestType", "improbable.codegen.TestType"), "TestType", new List<UnrealFieldDetails>(), unrealPackageDetails);
            var cleanTypeJob = new UnrealTypeJob(cleanUnrealTypeDetails, new HashSet<string>(), new HashSet<string>(), new HashSet<string>(), OutputDirectory, mockFileSystem);

            var dirtyUnrealTypeDetails = new UnrealTypeDetails(GenerateTypeDefinition("DirtyTestType", "improbable.codegen.DirtyTestType"), "DirtyTestType", new List<UnrealFieldDetails>(), unrealPackageDetails);
            var dirtyTypeJob = new UnrealTypeJob(dirtyUnrealTypeDetails, new HashSet<string>(), new HashSet<string>(), new HashSet<string>(), OutputDirectory, mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { cleanTypeJob, dirtyTypeJob }, new List<string>() { OutputDirectory });

            //Validate that the folder check was correct
            Assert.That(mockFileSystem.DeleteDirectoryCallCount == 0);
            Assert.That(mockFileSystem.GetFilesInDirectoryCallCount == 1);

            //Validate that the correct jobs were run and expected files were written
            Assert.That(mockFileSystem.GetFileInfoCallCount == 8);
            Assert.That(mockFileSystem.DirectoryExistsCallCount == 2);
            Assert.That(mockFileSystem.WriteToFileCallCount == 2);
            Assert.That(mockFileSystem.WrittenFiles.Count == 2);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "DirtyTestType.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "DirtyTestType.cpp")));

            Assert.That(cleanTypeJob.IsDirty() == false);
            Assert.That(dirtyTypeJob.IsDirty());
        }

        [Test]
        public void run_does_not_clean_if_output_folder_is_missing_one_of_the_output_files()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpatter, recursive) =>
            {
                return new List<IFile>()
                {
                    new MockFileWrapper(Path.Combine(path, "TestType.h"), path, new DateTime(1, 1, 1))
                };
            };

            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var fileMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));

                bool fileExists = false;
                var fileName = Path.GetFileName(path);
                if (fileName == "TestType.h")
                {
                    fileExists = true;
                }

                fileMock.ExistsMock = () => { return fileExists; };
                return fileMock;
            };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", TestSchemaPath, "improbable.codegen");

            var cleanUnrealTypeDetails = new UnrealTypeDetails(GenerateTypeDefinition("TestType", "improbable.codegen.TestType"), "TestType", new List<UnrealFieldDetails>(), unrealPackageDetails);
            var cleanTypeJob = new UnrealTypeJob(cleanUnrealTypeDetails, new HashSet<string>(), new HashSet<string>(), new HashSet<string>(), OutputDirectory, mockFileSystem);

            var dirtyUnrealTypeDetails = new UnrealTypeDetails(GenerateTypeDefinition("DirtyTestType", "improbable.codegen.DirtyTestType"), "DirtyTestType", new List<UnrealFieldDetails>(), unrealPackageDetails);
            var dirtyTypeJob = new UnrealTypeJob(dirtyUnrealTypeDetails, new HashSet<string>(), new HashSet<string>(), new HashSet<string>(), OutputDirectory, mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { cleanTypeJob, dirtyTypeJob }, new List<string>() { OutputDirectory });

            //Validate that the folder check was correct
            Assert.That(mockFileSystem.DeleteDirectoryCallCount == 0);
            Assert.That(mockFileSystem.GetFilesInDirectoryCallCount == 1);

            //Validate that the correct jobs were run and expected files were written
            Assert.That(mockFileSystem.GetFileInfoCallCount == 10);
            Assert.That(mockFileSystem.DirectoryExistsCallCount == 4);
            Assert.That(mockFileSystem.WriteToFileCallCount == 4);
            Assert.That(mockFileSystem.WrittenFiles.Count == 4);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestType.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestType.cpp")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "DirtyTestType.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "DirtyTestType.cpp")));

            Assert.That(cleanTypeJob.IsDirty());
            Assert.That(dirtyTypeJob.IsDirty());
        }

        [Test]
        public void run_does_clean_if_files_are_present_in_output_folder_that_should_not_be_there()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpatter, recursive) =>
            {
                return new List<IFile>()
                {
                    new MockFileWrapper(Path.Combine(path, "TestType.h"), path, new DateTime(1, 1, 1)),
                    new MockFileWrapper(Path.Combine(path, "TestType.cpp"), path, new DateTime(1, 1, 1)),
                    new MockFileWrapper(Path.Combine(path, "Invalid.cpp"), path, new DateTime(1, 1, 1)),
                };
            };
            mockFileSystem.DeleteDirectoryMock = (path) => { };
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var fileMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));

                bool fileExists = false;
                var fileName = Path.GetFileName(path);
                if (fileName == "TestType.h" || fileName == "TestType.cpp" || fileName == "Invalid.cpp")
                {
                    fileExists = true;
                }

                fileMock.ExistsMock = () => { return fileExists; };
                return fileMock;
            };

            var unrealPackageDetails = new UnrealPackageDetails("improbable.codegen.TestComponent", TestSchemaPath, "improbable.codegen");

            var cleanUnrealTypeDetails = new UnrealTypeDetails(GenerateTypeDefinition("TestType", "improbable.codegen.TestType"), "TestType", new List<UnrealFieldDetails>(), unrealPackageDetails);
            var cleanTypeJob = new UnrealTypeJob(cleanUnrealTypeDetails, new HashSet<string>(), new HashSet<string>(), new HashSet<string>(), OutputDirectory, mockFileSystem);

            var dirtyUnrealTypeDetails = new UnrealTypeDetails(GenerateTypeDefinition("DirtyTestType", "improbable.codegen.DirtyTestType"), "DirtyTestType", new List<UnrealFieldDetails>(), unrealPackageDetails);
            var dirtyTypeJob = new UnrealTypeJob(dirtyUnrealTypeDetails, new HashSet<string>(), new HashSet<string>(), new HashSet<string>(), OutputDirectory, mockFileSystem);

            var jobRunner = new JobRunner(mockFileSystem);
            jobRunner.Run(new List<ICodegenJob>() { cleanTypeJob, dirtyTypeJob }, new List<string>() { OutputDirectory });

            //Validate that the folder check was correct
            Assert.That(mockFileSystem.DeleteDirectoryCallCount == 1);
            Assert.That(mockFileSystem.GetFilesInDirectoryCallCount == 1);

            //Validate that the correct jobs were run and expected files were written
            Assert.That(mockFileSystem.GetFileInfoCallCount == 4);
            Assert.That(mockFileSystem.DirectoryExistsCallCount == 4);
            Assert.That(mockFileSystem.WriteToFileCallCount == 4);
            Assert.That(mockFileSystem.WrittenFiles.Count == 4);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestType.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestType.cpp")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "DirtyTestType.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "DirtyTestType.cpp")));

            Assert.That(cleanTypeJob.IsDirty());
            Assert.That(dirtyTypeJob.IsDirty());
        }

        private static MockFileSystem GenerateMockFileSystem()
        {
            var mockFileSystem = new MockFileSystem();
            mockFileSystem.DirectoryExistsMock = (directory) => { return true; };
            mockFileSystem.WriteToFileMock = (path, content) => { };

            return mockFileSystem;
        }

        private static ComponentDefinitionRaw GenerateComponentDefinition()
        {
            var componentDefinition = new ComponentDefinitionRaw();
            componentDefinition.id = "1";
            componentDefinition.name = "TestComponent";
            componentDefinition.qualifiedName = "improbable.codegen.TestComponent";
            componentDefinition.dataDefinition = new TypeReferenceRaw { builtInType = "Type" };
            return componentDefinition;
        }

        private static ComponentDefinitionRaw.CommandDefinitionRaw GenerateCommandDefinition()
        {
            var commandDefinition = new ComponentDefinitionRaw.CommandDefinitionRaw();
            commandDefinition.name = "TestCommand";
            commandDefinition.requestType = new TypeReferenceRaw() { builtInType = "int32" };
            commandDefinition.responseType = new TypeReferenceRaw() { builtInType = "int32" };
            return commandDefinition;
        }

        private static TypeDefinitionRaw GenerateTypeDefinition(string typeName, string qualifiedName)
        {
            var typeDefinition = new TypeDefinitionRaw();
            typeDefinition.name = typeName;
            typeDefinition.qualifiedName = qualifiedName;
            return typeDefinition;
        }

        private static TypeReferenceRaw GenerateTypeReferenceRaw()
        {
            var typeReferenceRaw = new TypeReferenceRaw();
            typeReferenceRaw.builtInType = "int32";
            return typeReferenceRaw;
        }

        private static EnumDefinitionRaw GenerateEnumDefinition()
        {
            var enumDefinitionRaw = new EnumDefinitionRaw();
            enumDefinitionRaw.name = "TestEnum";
            enumDefinitionRaw.qualifiedName = "improbable.codegen.TestEnum";
            enumDefinitionRaw.valueDefinitions = new EnumDefinitionRaw.ValueDefinitionRaw[] { };

            return enumDefinitionRaw;
        }
    }
}
