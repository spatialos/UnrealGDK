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

            Fields = Fields.Where(f =>
            {
                switch (f.TypeSelector)
                {
                    case FieldType.Option:
                        return f.OptionType.InnerType.ValueTypeSelector == ValueType.Primitive && f.OptionType.InnerType.Primitive == PrimitiveType.Entity;
                    case FieldType.List:
                        return f.ListType.InnerType.ValueTypeSelector == ValueType.Primitive && f.ListType.InnerType.Primitive == PrimitiveType.Entity;
                    case FieldType.Map:
                        return f.MapType.KeyType.ValueTypeSelector == ValueType.Primitive && f.MapType.KeyType.Primitive == PrimitiveType.Entity || f.MapType.ValueType.ValueTypeSelector == ValueType.Primitive && f.MapType.ValueType.Primitive == PrimitiveType.Entity;
                    case FieldType.Singular:
                        return f.SingularType.Type.ValueTypeSelector == ValueType.Primitive && f.SingularType.Type.Primitive == PrimitiveType.Entity;
                    default:
                        return false;
                }
            }).ToList();

            Annotations = component != null ? component.Annotations : bundle.Types[qualifiedName].Annotations;
            Events = component?.Events;
            OuterType = component != null ? "" : bundle.Types[qualifiedName].OuterType;
            //try
            //{
            //    OuterType = bundle.Types[qualifiedName].OuterType;

            //}
            //catch (Exception exception)
            //{
            //    Console.Error.WriteLine(exception);
            //    Environment.ExitCode = 1;
            //}
        }
    }
}
