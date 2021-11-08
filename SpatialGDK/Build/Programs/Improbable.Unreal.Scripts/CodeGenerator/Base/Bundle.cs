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

        public IReadOnlyDictionary<string, string> TypeToFileName { get; }

        public IReadOnlyDictionary<string, SourceReference> TypeToSourceReference { get; }

        public Bundle(SchemaBundle bundle)
        {
            SchemaBundle = bundle;

            Components = bundle.SchemaFiles.SelectMany(f => f.Components).ToDictionary(c => c.QualifiedName, c => c);
            Types = bundle.SchemaFiles.SelectMany(f => f.Types).ToDictionary(t => t.QualifiedName, t => t);
            Enums = bundle.SchemaFiles.SelectMany(f => f.Enums).ToDictionary(t => t.QualifiedName, t => t);

            Dictionary<string, string> fileNameDict = new Dictionary<string, string>(); ;
            TypeToFileName = fileNameDict;

            Dictionary<string, string> sourceRefDict = new Dictionary<string, SourceReference>();
            TypeToSourceReference = sourceRefDict;

            foreach (SchemaFile file in bundle.SchemaFiles)
            {
                foreach (TypeDefinition type in file.Types)
                {
                    fileNameDict[type.QualifiedName] = file.CanonicalPath;
                }

                foreach (ComponentDefinition type in file.Components)
                {
                    fileNameDict[type.QualifiedName] = file.CanonicalPath;
                }

                foreach (EnumDefinition type in file.Enums)
                {
                    fileNameDict[type.QualifiedName] = file.CanonicalPath;
                }
            }
        }

        public List<string> GetNestedTypes(string identifier)
        {
            return Types.Where(t =>
                    t.Key.StartsWith(identifier) &&
                    t.Key.Count(c => c == '.') == identifier.Count(c => c == '.') + 1)
                .Select(kv => kv.Key)
                .ToList();
        }

        public List<string> GetNestedEnums(string identifier)
        {
            return Enums.Where(t =>
                    t.Key.StartsWith(identifier) &&
                    t.Key.Count(c => c == '.') == identifier.Count(c => c == '.') + 1)
                .Select(kv => kv.Key)
                .ToList();
        }

        public HashSet<string> GetCommandTypes()
        {
            return new HashSet<string>(SchemaBundle.SchemaFiles.SelectMany(f => f.Components)
                .SelectMany(c => c.Commands)
                .SelectMany(cmd => new[] { cmd.RequestType, cmd.ResponseType }));
        }
    }
}
