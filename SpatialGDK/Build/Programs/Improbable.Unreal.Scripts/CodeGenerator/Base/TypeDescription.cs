using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Improbable.CodeGen.Base;
using ValueType = Improbable.CodeGen.Base.ValueType;

namespace Improbable.CodeGen.Base
{
    [DebuggerDisplay("{" + nameof(QualifiedName) + "}")]
    public readonly struct TypeDescription
    {
        public readonly string Namespace;

        public readonly string Name;

        public readonly string QualifiedName;

        public readonly SourceReference SourceReference;

        public readonly IReadOnlyList<TypeDescription> NestedTypes;

        public readonly IReadOnlyList<EnumDefinition> NestedEnums;

        public readonly IReadOnlyList<FieldDefinition> Fields;

        public readonly IReadOnlyList<Annotation> Annotations;

        public readonly IReadOnlyList<ComponentDefinition.EventDefinition> Events;

        public readonly uint? ComponentId;

        public readonly string OuterType;

        public TypeDescription(string qualifiedName, Bundle bundle)
        {
            Name = Text.SnakeCaseToPascalCase(qualifiedName).Split('.').Last();
            Namespace = $"{Text.GetNamespaceFromTypeName(qualifiedName)}";
            QualifiedName = qualifiedName;

            var directNestedTypes = bundle.GetNestedTypes(qualifiedName);
            var directNestedEnums = bundle.GetNestedEnums(qualifiedName);

            NestedTypes = directNestedTypes.Select(id =>
            {
                var t = bundle.Types[id];
                return new TypeDescription(t.QualifiedName, bundle);
            }).ToList();

            NestedEnums = directNestedEnums.Select(id => bundle.Enums[id]).ToList();

            bundle.Components.TryGetValue(qualifiedName, out var component);
            ComponentId = component?.ComponentId;

            if (ComponentId.HasValue)
            {
                SourceReference = bundle.Components[qualifiedName].SourceReference;
            }
            else
            {
                SourceReference = bundle.Types[qualifiedName].SourceReference;
            }

            Fields = component?.Fields;

            if (!string.IsNullOrEmpty(component?.DataDefinition))
            {
                // Inline fields into the component.
                Fields = bundle.Types[component.DataDefinition].Fields;
            }

            if (Fields == null)
            {
                if (ComponentId.HasValue)
                {
                    Fields = bundle.Components[qualifiedName].Fields;
                }
                else
                {
                    Fields = bundle.Types[qualifiedName].Fields;
                }
            }

            if (Fields == null)
            {
                throw new Exception("Internal error: no fields found");
            }

            Annotations = component != null ? component.Annotations : bundle.Types[qualifiedName].Annotations;
            Events = component?.Events;
            OuterType = component != null ? "" : bundle.Types[qualifiedName].OuterType;
        }
    }
}
