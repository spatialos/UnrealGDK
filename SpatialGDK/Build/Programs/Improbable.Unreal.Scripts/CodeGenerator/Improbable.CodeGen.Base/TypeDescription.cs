using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace Improbable.CodeGen.Base
{
    [DebuggerDisplay("{QualifiedName.QualifiedName}")]
    public readonly struct TypeDescription
    {
        private readonly Bundle bundle;

        public readonly string Namespace;

        public readonly string Name;

        public readonly string QualifiedName;

        public readonly IReadOnlyList<TypeDescription> NestedTypes;

        public readonly IReadOnlyList<EnumDefinition> NestedEnums;

        public readonly IReadOnlyList<FieldDefinition> Fields;

        public readonly IReadOnlyList<Annotation> Annotations;

        public readonly IReadOnlyList<ComponentDefinition.EventDefinition> Events;

        public readonly bool IsNestedType;

        public readonly bool IsComponent;
        public readonly uint? ComponentId;
        
        public TypeDescription(string qualifiedName, Bundle bundle)
        {
            this.bundle = bundle;

            Name = Text.SnakeCaseToPascalCase(qualifiedName).Split('.').Last();
            Namespace = $"{Text.GetNamespaceFromTypeName(qualifiedName)}";
            QualifiedName = qualifiedName;

            var directNestedTypes = bundle.GetNestedTypes(qualifiedName);
            var directNestedEnums = bundle.GetNestedEnums(qualifiedName);

            NestedTypes = directNestedTypes.Select(id =>
            {
                var t = bundle.Types[id];
                return new TypeDescription(t.Identifier.QualifiedName, bundle);
            }).ToList();

            NestedEnums = directNestedEnums.Select(id => bundle.Enums[id]).ToList();

            IsNestedType = bundle.IsNestedType(qualifiedName);

            IsComponent = bundle.Components.TryGetValue(qualifiedName, out var component);
            ComponentId = component?.ComponentId;

            Fields = component?.FieldDefinitions;
            if (component?.DataDefinition != null)
            {
                // Inline fields into the component.
                Fields = bundle.Types[component.DataDefinition.QualifiedName].FieldDefinitions;
            }

            if (Fields == null)
            {
                Fields = bundle.Types[qualifiedName].FieldDefinitions;
            }

            Annotations = component != null ? component.Annotations : bundle.Types[qualifiedName].Annotations;
            Events = component?.EventDefinitions;
        }
    }
}
