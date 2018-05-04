using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Test;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.Jobs
{
    internal class CodegenJobTest
    {
        [Test]
        public static void is_dirty_returns_true_if_the_time_stamp_of_the_latest_schema_is_newer_than_the_newest_output_file()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var fileName = Path.GetFileName(path);
                if (fileName == "TestSchema.schema")
                {
                    var schemaMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 2));
                    schemaMock.ExistsMock = () => { return true; };
                    return schemaMock;
                }

                var generalMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                generalMock.ExistsMock = () => { return true; };
                return generalMock;
            };

            var codegenJob = new StubCodegenJob(OutputDirectory, mockFileSystem, new List<string>() { "TestSchema.schema" }, new List<string>() { "TestOutput.h", "TestOutput.cpp" });

            Assert.That(codegenJob.IsDirty(), "Is dirty was not true when latest schema was newer than newest output file");
        }

        [Test]
        public static void is_dirty_returns_true_if_schema_files_input_are_empty()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var generalMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                generalMock.ExistsMock = () => { return true; };
                return generalMock;
            };

            var codegenJob = new StubCodegenJob(OutputDirectory, mockFileSystem, new List<string>(), new List<string>() { "TestOutput.h", "TestOutput.cpp" });

            Assert.That(codegenJob.IsDirty(), "Is dirty was not true when schema files input was empty");
        }

        [Test]
        public static void is_dirty_returns_true_if_there_are_no_files_in_output_folder()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var generalMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                generalMock.ExistsMock = () => { return true; };
                return generalMock;
            };

            var codegenJob = new StubCodegenJob(OutputDirectory, mockFileSystem, new List<string>() { "TestSchema.schema" }, new List<string>());

            Assert.That(codegenJob.IsDirty(), "Is dirty was not true when output files directory was empty");
        }

        [Test]
        public static void is_dirty_returns_false_if_any_of_the_output_files_is_newer_than_the_latest_schema()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var fileName = Path.GetFileName(path);
                if (fileName == "TestSchema.schema")
                {
                    var schemaMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                    schemaMock.ExistsMock = () => { return true; };
                    return schemaMock;
                }

                var generalMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 2));
                generalMock.ExistsMock = () => { return true; };
                return generalMock;
            };

            var codegenJob = new StubCodegenJob(OutputDirectory, mockFileSystem, new List<string>() { "TestSchema.schema" }, new List<string>() { "TestOutput.h", "TestOutput.cpp" });

            Assert.That(codegenJob.IsDirty() == false, "Is dirty was true when latest schema was older than newest output file");
        }

        [Test]
        public static void is_dirty_returns_true_if_not_all_output_files_of_a_job_are_present()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var fileName = Path.GetFileName(path);
                if (fileName == "TestOutput.h")
                {
                    var schemaMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                    schemaMock.ExistsMock = () => { return false; };
                    return schemaMock;
                }

                var generalMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                generalMock.ExistsMock = () => { return true; };
                return generalMock;
            };

            var codegenJob = new StubCodegenJob(OutputDirectory, mockFileSystem, new List<string>() { "TestSchema.schema" }, new List<string>() { "TestOutput.h", "TestOutput.cpp" });

            Assert.That(codegenJob.IsDirty(), "Is dirty was not true when one of the output files was missing");
        }

        [Test]
        public static void markasdirty_turns_the_job_into_dirty_state()
        {
            var mockFileSystem = GenerateMockFileSystem();

            var codegenJob = new StubCodegenJob(OutputDirectory, mockFileSystem, new List<string>() { "TestSchema.schema" }, new List<string>() { "TestOutput.h", "TestOutput.cpp" });
            codegenJob.MarkAsDirty();

            Assert.That(codegenJob.IsDirty(), "Is dirty was not true after being marked as true");
        }

        [Test]
        public static void run_creates_the_output_directory_if_it_does_not_exist()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var generalMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                generalMock.ExistsMock = () => { return true; };
                return generalMock;
            };
            var directoryCreated = false;
            var createdDirectory = string.Empty;
            mockFileSystem.CreateDirectoryMock = (path) =>
            {
                directoryCreated = true;
                createdDirectory = path;
            };
            mockFileSystem.DirectoryExistsMock = (path) =>
            {
                if (directoryCreated)
                {
                    return true;
                }

                return false;
            };

            var codegenJob = new StubCodegenJob(OutputDirectory, mockFileSystem, new List<string>() { "TestSchema.schema" }, new List<string>() { Path.Combine("CreatedDir", "TestOutput.h"), Path.Combine("CreatedDir", "TestOutput.cpp") });
            codegenJob.Run();

            Assert.That(mockFileSystem.DirectoryExistsCallCount == 2);
            Assert.That(mockFileSystem.CreateDirectoryCallCount == 1);
            Assert.That(createdDirectory == Path.Combine(OutputDirectory, "CreatedDir"));
        }

        [Test]
        public static void run_writes_the_expected_files_to_disk()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var generalMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                generalMock.ExistsMock = () => { return true; };
                return generalMock;
            };

            var codegenJob = new StubCodegenJob(OutputDirectory, mockFileSystem, new List<string>() { "TestSchema.schema" }, new List<string>() { "TestOutput.h", "TestOutput.cpp" });
            codegenJob.Run();

            Assert.That(mockFileSystem.WriteToFileCallCount == 2);
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestOutput.h")));
            Assert.That(mockFileSystem.WrittenFiles.Contains(Path.Combine(OutputDirectory, "TestOutput.cpp")));
        }

        [Test]
        public static void clean_deletes_the_expected_files_on_disk()
        {
            var mockFileSystem = GenerateMockFileSystem();
            var mockFileWrappers = new List<MockFileWrapper>();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var generalMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                generalMock.ExistsMock = () => { return true; };
                generalMock.DeleteMock = () => { };
                mockFileWrappers.Add(generalMock);
                return generalMock;
            };
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpattern, recursive) => { return new List<IFile>() { new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1)) }; };

            var codegenJob = new StubCodegenJob(OutputDirectory, mockFileSystem, new List<string>() { "TestSchema.schema" }, new List<string>() { "TestOutput.h", "TestOutput.cpp" });
            codegenJob.Clean();

            Assert.That(mockFileWrappers.Count == 2);

            Assert.That(mockFileWrappers.Any(file => file.CompletePath == Path.Combine(OutputDirectory, "TestOutput.h")));
            Assert.That(mockFileWrappers.Any(file => file.CompletePath == Path.Combine(OutputDirectory, "TestOutput.cpp")));

            foreach (var fileMock in mockFileWrappers)
            {
                Assert.That(fileMock.ExistsCallCount == 1);
                Assert.That(fileMock.DeleteCallCount == 1);
            }
        }

        [Test]
        public static void clean_removes_empty_folders_in_output_directory_if_they_are_empty()
        {
            var mockFileSystem = GenerateMockFileSystem();
            mockFileSystem.GetFileInfoMock = (path) =>
            {
                var generalMock = new MockFileWrapper(path, Path.GetDirectoryName(path), new DateTime(1, 1, 1));
                generalMock.ExistsMock = () => { return true; };
                generalMock.DeleteMock = () => { };
                return generalMock;
            };

            var remainingFileCount = 2;
            mockFileSystem.GetFilesInDirectoryMock = (path, searchpattern, recursive) =>
            {
                remainingFileCount--;

                if (remainingFileCount > 0)
                {
                    return new List<IFile>() { new MockFileWrapper("filetodelete", "filetodelete", new DateTime(1, 1, 1)) };
                }

                return new List<IFile>();
            };
            var deletedDirectory = string.Empty;
            mockFileSystem.DeleteDirectoryMock = (path) => { deletedDirectory = path; };

            var codegenJob = new StubCodegenJob(OutputDirectory, mockFileSystem, new List<string>() { "TestSchema.schema" }, new List<string>() { Path.Combine("FolderToDelete", "TestOutput.h"), Path.Combine("FolderToDelete", "TestOutput.cpp") });
            codegenJob.Clean();

            Assert.That(mockFileSystem.GetFilesInDirectoryCallCount == 2);
            Assert.That(mockFileSystem.DeleteDirectoryCallCount == 1);
            Assert.That(deletedDirectory == Path.Combine(OutputDirectory, "FolderToDelete"));
        }

        private static MockFileSystem GenerateMockFileSystem()
        {
            var mockFileSystem = new MockFileSystem();
            mockFileSystem.DirectoryExistsMock = (directory) => { return true; };
            mockFileSystem.WriteToFileMock = (path, content) => { };

            return mockFileSystem;
        }

        private static string OutputDirectory = Path.Combine("OutputDir", "test");
    }
}
