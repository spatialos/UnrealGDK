// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using CommandLine;
using Improbable.Codegen.Base;
using Improbable.CodeGen.Base;
using Improbable.CodeGen.Unreal;

namespace CodeGenerator
{
    public class Options
    {
        [Option("input-bundle", Required = true,
            HelpText = "The path to the JSON Bundle file output by the SpatialOS schema_compiler.")]
        public string InputBundle { get; set; }

        [Option("output-dir", Required = true,
            HelpText = "The path to write the generated code to.")]
        public string OutputDir { get; set; }
    }

    internal class Program
    {
        static readonly uint ExternalComponentIdLowerBound = 1000;
        static readonly uint ExternalComponentIdUpperBound = 20000000;

        private static void Main(string[] args)
        {
            Parser.Default.ParseArguments<Options>(args)
                .WithParsed(Run)
                .WithNotParsed(errors =>
                {
                    foreach (var error in errors)
                    {
                        Console.Error.WriteLine(error);
                    }

                    Environment.ExitCode = 1;
                });
        }

        private static void Run(Options options)
        {
            try
            {
                var bundle = SchemaBundleLoader.LoadBundle(options.InputBundle);

                ValidateBundle(bundle);

                var generators = new List<ICodeGenerator>
                {
                    new UnrealGenerator()
                };

                var output = new List<GeneratedFile>();

                generators.ForEach((ICodeGenerator g) => output.AddRange(g.GenerateFiles(bundle)));

                foreach (var generatedFile in output)
                {
                    WriteFile(options.OutputDir, generatedFile);
                }
            }
            catch (Exception exception)
            {
                Console.Error.WriteLine(exception);
                Environment.ExitCode = 1;
            }
        }

        private static void ValidateBundle(Bundle bundle)
        {
            foreach (var component in bundle.Components)
            {
                if (component.Value.ComponentId < ExternalComponentIdLowerBound || component.Value.ComponentId > ExternalComponentIdUpperBound)
                {
                    throw new Exception($@"External schema component IDs must be in the range {ExternalComponentIdLowerBound}-{ExternalComponentIdUpperBound}
Component {component.Value.QualifiedName} has ID: {component.Value.ComponentId}");
                }
            }
        }

        private static void WriteFile(string codegenOutputFolder, GeneratedFile file)
        {
            var generatedFilePath = Path.Combine(codegenOutputFolder, file.RelativeFilePath);
            var generatedFileDirectory = Path.GetDirectoryName(generatedFilePath);
            if (!Directory.Exists(generatedFileDirectory))
            {
                Directory.CreateDirectory(generatedFileDirectory);
            }

            File.WriteAllText(generatedFilePath, file.Contents, Encoding.UTF8);
        }     
    }
}
