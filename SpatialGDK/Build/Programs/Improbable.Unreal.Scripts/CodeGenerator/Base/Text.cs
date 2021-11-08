using System;
using System.Collections.Generic;
using System.Linq;

namespace Improbable.CodeGen.Base
{
    public static class Text
    {
        public static string CapitalizeFirstLetter(string text)
        {
            return char.ToUpperInvariant(text[0]) + text.Substring(1, text.Length - 1);
        }

        public static string ToPascalCase(IEnumerable<string> parts)
        {
            string result = string.Empty;
            foreach (string s in parts)
            {
                result = result + CapitalizeFirstLetter(s);
            }

            return result;
        }

        public static string ToSnakeCase(string text)
        {
            return text.ToLower().Replace(".", "_");
        }

        public static string SnakeCaseToPascalCase(string text)
        {
            return ToPascalCase(text.Split(new[] {"_"}, StringSplitOptions.RemoveEmptyEntries));
        }

        public static string SnakeCaseToCamelCase(string text)
        {
            string[] parts = text.Split(new[] {"_"}, StringSplitOptions.RemoveEmptyEntries);
            return parts[0] + ToPascalCase(parts.Skip(1));
        }

        public static string CapitalizeNamespace(string text)
        {
            string[] strings = text.Split(new[] {"."}, StringSplitOptions.RemoveEmptyEntries);
            return string.Join(".", strings.Select(SnakeCaseToPascalCase));
        }

        public static string[] GetNamespaceFromTypeName(string text)
        {
            string[] strings = text.ToLower().Split('.');
            return strings.Take(strings.Length - 1).ToArray();
        }

        public static string Indent(int level, string inputString)
        {
            string indent = string.Empty.PadLeft(level, '\t');
            return indent + inputString.Replace("\n", $"\n{indent}");
        }

        public static string ReplacesDotsWithDoubleColons(string identifier)
        {
            return $"{identifier.Replace(".", "::")}";
        }
    }
}
