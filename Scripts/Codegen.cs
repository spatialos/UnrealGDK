using System.IO;
using System.Linq;

namespace Improbable
{
    public static class Codegen
    {
        public static void Main(string[] args)
        {
// IMPROBABLE: giray changing hardcoded Game folder to Scavengers
            Common.EnsureDirectoryEmpty(@"Scavengers\Source\SpatialGDK\Generated\Cpp");
            Common.EnsureDirectoryEmpty(@"Scavengers\Intermediate\Improbable\Json");

            var files = Directory.GetFiles(@"spatial\schema", "*.schema", SearchOption.AllDirectories).Union(
                Directory.GetFiles(@"Scavengers\Binaries\ThirdParty\Improbable\Programs\schema", "*.schema",
                    SearchOption.AllDirectories));

            var arguments = new[]
            {
                @"--cpp_out=Scavengers\Source\SpatialGDK\Generated\Cpp",
                @"--ast_json_out=Scavengers\Intermediate\Improbable\Json",
                @"--schema_path=spatial\schema",
                @"--schema_path=Scavengers\Binaries\ThirdParty\Improbable\Programs\schema"
            }.Union(files);

            Common.RunRedirected(@"Scavengers\Binaries\ThirdParty\Improbable\Programs\schema_compiler.exe", arguments);
            Common.RunRedirected(@"Scavengers\Binaries\ThirdParty\Improbable\Programs\UnrealCodeGenerator.exe", new[]
            {
                @"--json-dir=Scavengers/Intermediate/Improbable/Json",
                @"--output-dir=Scavengers/Source/SpatialGDK/Generated/UClasses"
            });
        }
    }
}