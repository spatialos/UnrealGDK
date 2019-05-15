using Improbable.Codegen.Base;
using Improbable.CodeGen.Base;
using System.Collections.Generic;

namespace Improbable.CodeGen.Unreal
{
    public static class MapEquals
    {
        public static string HeaderName = "MapEquals.h";

        public static List<GeneratedFile> GenerateMapEquals()
        {
            return new List<GeneratedFile>
            {
                new GeneratedFile(HeaderName, GenerateHeader()),
            };
        }

        private static string GenerateHeader()
        {
            return $@"#pragma once

template<typename k, typename v>
bool operator==(TMap<k, v> Map1, TMap<k, v> Map2)
{{
{Text.Indent(1, $@"if (Map1.Num() != Map2.Num())
{{
{Text.Indent(1, "return false;")}
}}
for (const TPair<k, v>& Elem : Map1)
{{
{Text.Indent(1, $@"if (Elem.Value != Map2.FindRef(Elem.Key))
{{
{Text.Indent(1, "return false;")}
}}")}
}}
return true;")}
}}
";
        }
    }
}
