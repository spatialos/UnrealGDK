using System;
using System.Collections.Generic;
using System.Linq;
using Improbable.CodeGeneration.FileHandling;
using Improbable.CodeGeneration.Jobs;
using Improbable.Unreal.CodeGeneration.Jobs;

namespace Improbable.Unreal.CodeGeneration
{
    public class UnrealCodeGenerator
    {
        private readonly CodeGeneratorOptions options;
        private readonly IFileSystem fileSystem;

        public static int Main(string[] args)
        {
            var result = 1;
            try
            {
                var options = CodeGeneratorOptions.ParseArguments(args);
                var generator = new UnrealCodeGenerator(options, new FileSystem());

                result = generator.Run();
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Code generation failed with exception: {0}", e.Message);
                if (e.InnerException != null)
                {
                    Console.Error.WriteLine(e.InnerException);
                }

                Console.Error.WriteLine(e.StackTrace);
            }

            return result;
        }

        public UnrealCodeGenerator(CodeGeneratorOptions options, IFileSystem fileSystem)
        {
            this.options = options;
            this.fileSystem = fileSystem;
        }

        public int Run()
        {
            if (!ValidateOptions())
            {
                ShowHelpMessage();
                return 1;
            }

            if (options.ShouldShowHelp)
            {
                ShowHelpMessage();
                return 0;
            }

            var jobGenerator = new JobGenerator(fileSystem);
            var schemaFilesRaw = SchemaFiles.GetSchemaFilesRaw(options.JsonDirectory, fileSystem)
                                            .Where(f => string.IsNullOrEmpty(options.Package) || f.package == options.Package)
                                            .ToList();
            var jobs = jobGenerator.GenerateJobs(schemaFilesRaw, options);

            var jobRunner = new JobRunner(fileSystem);
            jobRunner.Run(jobs, new List<string>() { options.OutputDir });

            return 0;
        }

        private void ShowHelpMessage()
        {
            Console.WriteLine("Usage: ");
            Console.WriteLine(options.HelpText);
        }

        private bool ValidateOptions()
        {
            if (options.JsonDirectory == null)
            {
                Console.WriteLine("Input directory not specified.");
                return false;
            }

            if (!fileSystem.DirectoryExists(options.JsonDirectory))
            {
                Console.WriteLine("Not generating code since no JSON schema exists in " + options.JsonDirectory);
                return false;
            }

            return true;
        }
    }
}
