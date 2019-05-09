using System;
using System.Collections.Generic;
using System.Linq;

namespace Improbable.CodeGen.Base
{
    public class Bundle
    {
        public SchemaBundle SchemaBundle { get; }
        public IReadOnlyDictionary<string, TypeDefinition> Types { get; }
        public IReadOnlyDictionary<string, EnumDefinition> Enums { get; }
        public IReadOnlyDictionary<string, ComponentDefinition> Components { get; }

        public Bundle(SchemaBundle bundle)
        {
            SchemaBundle = bundle;

            Components = bundle.V1.ComponentDefinitions.ToDictionary(c => c.Identifier.QualifiedName, c => c);
            Types = bundle.V1.TypeDefinitions.ToDictionary(t => t.Identifier.QualifiedName, t => t);
            Enums = bundle.V1.EnumDefinitions.ToDictionary(t => t.Identifier.QualifiedName, t => t);
        }

        public List<string> GetNestedTypes(string qualifiedName)
        {
            return Types.Where(t =>
                    t.Key.StartsWith($"{qualifiedName}.") &&
                    t.Key.Count(c => c == '.') == qualifiedName.Count(c => c == '.') + 1)
                .Select(kv => kv.Key)
                .ToList();
        }

        public List<string> GetNestedEnums(string qualifiedName)
        {
            return Enums.Where(t =>
                    t.Key.StartsWith($"{qualifiedName}.") &&
                    t.Key.Count(c => c == '.') == qualifiedName.Count(c => c == '.') + 1)
                .Select(kv => kv.Key)
                .ToList();
        }

        public bool IsNestedType(string qualifiedName)
        {
            return Types.Any(type => type.Key.Equals(qualifiedName.Substring(0, qualifiedName.LastIndexOf("."))));
        }

        public bool IsNestedEnum(string qualifiedName)
        {
            return IsNestedType(qualifiedName);
        }

        public string GetOutermostTypeWrapperForType(string qualifiedName)
        {
            var qualifiedNameSplit = qualifiedName.Split('.');
            for (var i = 1; i < qualifiedNameSplit.Count(); ++i)
            {
                var candidateType = string.Join(".", qualifiedNameSplit.Take(i));
                if (Types.Any(type => type.Key.Equals(candidateType))) {
                    return candidateType;
                }
            }
            return qualifiedName;
        }

        public HashSet<string> GetCommandTypes()
        {
            return new HashSet<string>(SchemaBundle.V1.ComponentDefinitions
                .SelectMany(c => c.CommandDefinitions)
                .SelectMany(cmd => new [] { cmd.RequestType, cmd.ResponseType })
                .Select(t => t.Type.QualifiedName));
        }
    }
}
