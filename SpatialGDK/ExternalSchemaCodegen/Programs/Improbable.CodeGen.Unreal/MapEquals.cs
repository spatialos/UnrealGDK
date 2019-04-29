using Improbable.Codegen.Base;
using Improbable.CodeGen.Base;
using System;
using System.Collections.Generic;
using System.Text;

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
            var builder = new StringBuilder();

            builder.AppendLine("#pragma once");
            builder.Append(Environment.NewLine);
            builder.AppendLine("template<typename k, typename v>");
            builder.AppendLine("bool operator==(TMap<k, v> Map1, TMap<k, v> Map2)");
            builder.AppendLine("{");
            builder.AppendLine(Text.Indent(1, "if (Map1.Num() != Map2.Num())"));
            builder.AppendLine(Text.Indent(1, "{"));
            builder.AppendLine(Text.Indent(2, "return false;"));
            builder.AppendLine(Text.Indent(1, "}"));
            builder.Append(Environment.NewLine);
            builder.AppendLine(Text.Indent(1, "for (const TPair<k, v>& Elem : Map1)"));
            builder.AppendLine(Text.Indent(1, "{"));
            builder.AppendLine(Text.Indent(2, "if (Elem.Value != Map2.FindRef(Elem.Key))"));
            builder.AppendLine(Text.Indent(2, "{"));
            builder.AppendLine(Text.Indent(3, "return false;"));
            builder.AppendLine(Text.Indent(2, "}"));
            builder.AppendLine(Text.Indent(1, "}"));
            builder.Append(Environment.NewLine);
            builder.AppendLine(Text.Indent(1, "return true;"));
            builder.AppendLine("}");
            builder.Append(Environment.NewLine);

            return builder.ToString();
        }
    }
}
