using System.IO;
using System.Linq;

namespace Improbable
{
    public static class Codegen
    {
        public static void Main(string[] args)
        {
            Common.EnsureDirectoryEmpty(@"Intermediate\Improbable\Json");

            // Back compat: ensure that the standard schema is available for the `spatial upload` command.
            // It's distributed with the CodeGenerator, so it's copied from there into the expected location.
            Common.RunRedirected(@"Build\Scripts\DiffCopy.bat", new[]
            {
                @"Binaries\ThirdParty\Improbable\Programs\schema",
                @"..\..\..\spatial\build\dependencies\schema\standard_library"
            });

            var files = Directory.GetFiles(@"..\..\..\spatial\schema", "*.schema", SearchOption.AllDirectories).Union(
                Directory.GetFiles(@"Binaries\ThirdParty\Improbable\Programs\schema", "*.schema",
                    SearchOption.AllDirectories));

            string intermediateSchemaCompilerDirectory = Path.GetFullPath(Path.Combine("Intermediate/Improbable/", Path.GetRandomFileName()));
            Directory.CreateDirectory(intermediateSchemaCompilerDirectory);
            var arguments = new[]
            {
                $"--cpp_out=\"{intermediateSchemaCompilerDirectory}\"",
                @"--ast_json_out=Intermediate\Improbable\Json",
                @"--schema_path=..\..\..\spatial\schema",
                @"--schema_path=Binaries\ThirdParty\Improbable\Programs\schema"
            }.Union(files);

            Common.RunRedirected(@"Binaries\ThirdParty\Improbable\Programs\schema_compiler.exe", arguments);

            {
                // TODO: remove this when WRK-416 is implemented (specify .cpp extension to schema_compiler)
                var generatedFiles = new DirectoryInfo(intermediateSchemaCompilerDirectory);
                foreach (var file in generatedFiles.GetFiles("*.cc", SearchOption.AllDirectories))
                {
                    File.Move(file.FullName, Path.ChangeExtension(file.FullName, ".cpp"));
                }
            }

            Common.RunRedirected(@"Build\Scripts\DiffCopy.bat", new[]
            {
                $"\"{intermediateSchemaCompilerDirectory}\"",
                @"Source\SpatialGDK\Generated\Cpp",
                @"--remove-input"
            });

            string intermediateUnrealCodegenDirectory = Path.Combine("Intermediate/Improbable/", Path.GetRandomFileName());
            Directory.CreateDirectory(intermediateUnrealCodegenDirectory);
            Common.RunRedirected(@"Binaries\ThirdParty\Improbable\Programs\UnrealCodeGenerator.exe", new[]
            {
                @"--json-dir=Intermediate/Improbable/Json",
                $"--output-dir=\"{intermediateUnrealCodegenDirectory}\""
            });
            Common.RunRedirected(@"Build\Scripts\DiffCopy.bat", new[]
            {
                $"\"{intermediateUnrealCodegenDirectory}\"",
                @"Source/SpatialGDK/Generated/UClasses",
                @"--remove-input"
            });
        }
    }
}
