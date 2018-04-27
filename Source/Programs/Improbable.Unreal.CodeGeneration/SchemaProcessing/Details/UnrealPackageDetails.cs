using System;
using System.Linq;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.Details
{
    public class UnrealPackageDetails
    {
        public readonly string NamespacePrefix;
        public readonly string Include;
        public readonly string SourceSchema;
        public readonly string CapitalisedCamelCase;

        public UnrealPackageDetails(string canonicalName, string completePath, string package)
        {
            NamespacePrefix = string.Format("{0}::", package.Replace(".", "::"));
            Include = canonicalName.Replace(".schema", ".h");
            SourceSchema = completePath;
            var parts = package.Split(new[] { "." }, StringSplitOptions.RemoveEmptyEntries)
                               .Select<string, string>(s => char.ToUpperInvariant(s[0]) + s.Substring(1, s.Length - 1)).ToArray();
            CapitalisedCamelCase = string.Join(string.Empty, parts);
        }
    }
}
