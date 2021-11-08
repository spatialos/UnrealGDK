using Improbable.Codegen.Base;
using Improbable.CodeGen.Base;
using System.Collections.Generic;
using System.Linq;

namespace Improbable.CodeGen.Unreal
{
    public class UnrealGenerator : ICodeGenerator
    {
        public static string GeneratorTitle = "Unreal External Schema Codegen";

        public List<GeneratedFile> GenerateFiles(Bundle bundle)
        {
            List<GeneratedFile> generatedFiles = new List<GeneratedFile>();
            Dictionary<string, TypeGeneratedCode> allGeneratedTypeContent = new Dictionary<string, TypeGeneratedCode>();
            List<TypeDescription> types = bundle.Types.Select(kv => new TypeDescription(kv.Key, bundle))
                .Union(bundle.Components.Select(kv => new TypeDescription(kv.Key, bundle)))
                .ToList();
            IEnumerable<TypeDescription> topLevelTypes = types.Where(type => type.OuterType.Equals(""));
            IEnumerable<KeyValuePair<string, EnumDefinition>> topLevelEnums = bundle.Enums.Where(_enum => _enum.Value.OuterType.Equals(""));

            // Generate utility files
            generatedFiles.AddRange(HelperFunctions.GetHelperFunctionFiles());
            generatedFiles.AddRange(MapEquals.GenerateMapEquals());

            // Generate external schema interface
            generatedFiles.AddRange(InterfaceGenerator.GenerateInterface(types.Where(type => type.ComponentId.HasValue).ToList(), bundle));

            // Generated all type file content
            foreach (TypeDescription toplevelType in topLevelTypes)
            {
                generatedFiles.Add(new GeneratedFile(Types.TypeToHeaderFilename(toplevelType.QualifiedName), HeaderGenerator.GenerateHeader(toplevelType, types, allGeneratedTypeContent, bundle)));
                generatedFiles.Add(new GeneratedFile(Types.TypeToSourceFilename(toplevelType.QualifiedName), SourceGenerator.GenerateSource(toplevelType, types, allGeneratedTypeContent, bundle)));
            }

            // Add enum files to generated files, ignoring nested enum which are defined in parent files
            foreach (KeyValuePair<string, EnumDefinition> topLevelEnum in topLevelEnums)
            {
                generatedFiles.Add(new GeneratedFile(Types.TypeToHeaderFilename(topLevelEnum.Key), EnumGenerator.GenerateTopLevelEnum(topLevelEnum.Value, bundle)));
            }

            return generatedFiles;
        }
    }
}
