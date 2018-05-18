using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.IO;

namespace Improbable
{
    public static class Codegen
    {
        public static void Main(string[] args)
        {
            Common.EnsureDirectoryEmpty(@"Game\Source\SpatialGDK\Generated\Cpp");
            Common.EnsureDirectoryEmpty(@"Game\Intermediate\Improbable\Json");

            var files = Directory.GetFiles(@"spatial\schema", "*.schema", SearchOption.AllDirectories).
                        Union(Directory.GetFiles(@"Game\Binaries\ThirdParty\Improbable\Programs\schema", "*.schema", SearchOption.AllDirectories));

            var arguments = new [] {
                @"--cpp_out=Game\Source\SpatialGDK\Generated\Cpp",
                @"--ast_json_out=Game\Intermediate\Improbable\Json",
                @"--schema_path=spatial\schema",
                @"--schema_path=Game\Binaries\ThirdParty\Improbable\Programs\schema"
            }.Union(files);

            Common.RunRedirected(@"Game\Binaries\ThirdParty\Improbable\Programs\schema_compiler.exe", arguments);
            Common.RunRedirected(@"Game\Binaries\ThirdParty\Improbable\Programs\UnrealCodeGenerator.exe", new [] {
                @"--json-dir=Game/Intermediate/Improbable/Json",
                @"--output-dir=Game/Source/SpatialGDK/Generated/UClasses"
            });
        }
    }
}
