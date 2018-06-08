using System.IO;
using System.Linq;

namespace Improbable
{
    public static class Codegen
    {
        public static void Main(string[] args)
        {
            Common.EnsureDirectoryEmpty(@"Intermediate\Improbable\Json");

            // Copy the Schema dependencies from the worker sdk to the spatial folder.
            var spatialDependencyDir = new DirectoryInfo(@"..\spatial\build\dependencies\schema\standard_library");

            if(!spatialDependencyDir.Exists)
            {
                spatialDependencyDir.Delete(true);
                spatialDependencyDir.Create();
            }

            Common.RunRedirected(@"Scripts\DiffCopy.bat", new[]
            {
                @"Binaries\ThirdParty\Improbable\Programs\schema",
                $"{spatialDependencyDir.FullName}"
            });

            var files = Directory.GetFiles(@"..\spatial\schema", "*.schema", SearchOption.AllDirectories).Union(
                Directory.GetFiles(@"Binaries\ThirdParty\Improbable\Programs\schema", "*.schema",
                    SearchOption.AllDirectories));

            string intermediateSchemaCompilerDirectory = Path.GetFullPath(Path.Combine("Intermediate/Improbable/", Path.GetRandomFileName()));
            Directory.CreateDirectory(intermediateSchemaCompilerDirectory);
            var arguments = new[]
            {
                $"--cpp_out={intermediateSchemaCompilerDirectory}",
                @"--ast_json_out=Intermediate\Improbable\Json",
                @"--schema_path=..\spatial\schema",
                @"--schema_path=Binaries\ThirdParty\Improbable\Programs\schema"
            }.Union(files);

            Common.RunRedirected(@"Binaries\ThirdParty\Improbable\Programs\schema_compiler.exe", arguments);

            Common.RunRedirected(@"Scripts\DiffCopy.bat", new[]
            {
                $"{intermediateSchemaCompilerDirectory}",
                @"Source\SpatialGDK\Generated\Cpp"
            });

            string intermediateUnrealCodegenDirectory = Path.Combine("Intermediate/Improbable/", Path.GetRandomFileName());
            Directory.CreateDirectory(intermediateUnrealCodegenDirectory);
            Common.RunRedirected(@"Binaries\ThirdParty\Improbable\Programs\UnrealCodeGenerator.exe", new[]
            {
                @"--json-dir=Intermediate/Improbable/Json",
                $"--output-dir={intermediateUnrealCodegenDirectory}"
            });
            Common.RunRedirected(@"Scripts\DiffCopy.bat", new[]
            {
                $"{intermediateUnrealCodegenDirectory}",
                @"Source/SpatialGDK/Generated/UClasses"
            });
        }
    }
}
