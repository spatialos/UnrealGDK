using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Improbable.CodeGen.Base
{

    public enum PrimitiveType
    {
        Invalid = 0,
        Int32 = 1,
        Int64 = 2,
        Uint32 = 3,
        Uint64 = 4,
        Sint32 = 5,
        Sint64 = 6,
        Fixed32 = 7,
        Fixed64 = 8,
        Sfixed32 = 9,
        Sfixed64 = 10,
        Bool = 11,
        Float = 12,
        Double = 13,
        String = 14,
        EntityId = 15,
        Bytes = 16,
        Entity = 17
    }

    public enum ValueType
    {
        Enum,
        Primitive,
        Type
    }

    public class TypeReference
    {
        public string Enum;
        public PrimitiveType Primitive;
        public string Type;

        public ValueType ValueTypeSelector
        {
            get
            {
                if (Primitive != PrimitiveType.Invalid)
                {
                    return ValueType.Primitive;
                }

                if (Type != null)
                {
                    return ValueType.Type;
                }

                if (Enum != null)
                {
                    return ValueType.Enum;
                }

                throw new InvalidOperationException("TypeReference doesn't have any type set.");
            }
        }
    }

    public class Value
    {
        public SourceReference SourceReference;

        public bool BoolValue;
        public string BytesValue;
        public double DoubleValue;
        public long EntityIdValue;
        public float FloatValue;
        public int Int32Value;
        public long Int64Value;
        public ListValueHolder ListValue;
        public MapValueHolder MapValue;
        public OptionValueHolder OptionValue;
        public SchemaEnumValue EnumValue;
        public string StringValue;
        public TypeValue TypeValue;
        public uint Uint32Value;
        public ulong Uint64Value;

        public class OptionValueHolder
        {
            public Value Value;
        }

        public class ListValueHolder
        {
            public IReadOnlyList<Value> Values;
        }

        public class MapValueHolder
        {
            public IReadOnlyList<MapPairValue> Values;

            public class MapPairValue
            {
                public Value Key;
                public Value Value;
            }
        }
    }

    public class SchemaEnumValue
    {
        public string Enum;
        public string EnumValue;

        public string Name;
        public string Value;
    }

    public class TypeValue
    {
        public IReadOnlyList<FieldValue> Fields;

        public string Type;

        public class FieldValue
        {
            public SourceReference SourceReference;
            public string Name;
            public Value Value;
        }
    }

    public class Annotation
    {
        public SourceReference SourceReference;
        public TypeValue TypeValue;
    }

    public class EnumValueDefinition
    {
        public SourceReference SourceReference;
        public IReadOnlyList<Annotation> Annotations;
        public string Name;

        public uint Value;
    }

    [DebuggerDisplay("{" + nameof(QualifiedName) + "}")]
    public class EnumDefinition
    {
        public SourceReference SourceReference;
        public IReadOnlyList<Annotation> Annotations;
        public string QualifiedName;
        public string Name;
        public string OuterType;
        public IReadOnlyList<EnumValueDefinition> Values;
    }

    public enum FieldType
    {
        Option,
        List,
        Map,
        Singular
    }

    [DebuggerDisplay("{" + nameof(Name) + "}" + " ({" + nameof(FieldId) + "})")]
    public class FieldDefinition
    {
        public SourceReference SourceReference;

        public IReadOnlyList<Annotation> Annotations;

        public uint FieldId;

        public string Name;

        public ListTypeRef ListType;
        public MapTypeRef MapType;
        public OptionTypeRef OptionType;

        public SingularTypeRef SingularType;

        public bool Transient;

        public FieldType TypeSelector
        {
            get
            {
                if (SingularType != null)
                {
                    return FieldType.Singular;
                }

                if (OptionType != null)
                {
                    return FieldType.Option;
                }

                if (ListType != null)
                {
                    return FieldType.List;
                }

                if (MapType != null)
                {
                    return FieldType.Map;
                }

                throw new InvalidOperationException("FieldType has no types set.");
            }
        }

        public class SingularTypeRef
        {
            public TypeReference Type;
        }

        public class OptionTypeRef
        {
            public TypeReference InnerType;
        }

        public class ListTypeRef
        {
            public TypeReference InnerType;
        }

        public class MapTypeRef
        {
            public TypeReference KeyType;
            public TypeReference ValueType;
        }
    }

    [DebuggerDisplay("{" + nameof(QualifiedName) + "}")]
    public class TypeDefinition
    {
        public SourceReference SourceReference;

        public IReadOnlyList<Annotation> Annotations;
        public IReadOnlyList<FieldDefinition> Fields;
        public string QualifiedName;
        public string Name;
        public string OuterType;
    }

    [DebuggerDisplay("{" + nameof(QualifiedName) + "} {" + nameof(ComponentId) + "}")]
    public class ComponentDefinition
    {
        public SourceReference SourceReference;

        public IReadOnlyList<Annotation> Annotations;
        public IReadOnlyList<CommandDefinition> Commands;
        public uint ComponentId;

        public string DataDefinition;
        public IReadOnlyList<EventDefinition> Events;

        public IReadOnlyList<FieldDefinition> Fields;

        public string QualifiedName;
        public string Name;

        [DebuggerDisplay("{" + nameof(QualifiedName) + "}")]
        public class EventDefinition
        {
            public SourceReference SourceReference;

            public IReadOnlyList<Annotation> Annotations;
            public uint EventIndex;
            public string Name;
            public string Type;
        }

        [DebuggerDisplay("{" + nameof(QualifiedName) + "}" + " {" + nameof(CommandIndex) + "}")]
        public class CommandDefinition
        {
            public SourceReference SourceReference;

            public IReadOnlyList<Annotation> Annotations;
            public uint CommandIndex;

            public string Name;
            public string RequestType;
            public string ResponseType;
        }
    }

    public class SourceReference
    {
        public uint Column;
        public uint Line;
    }

    public class SchemaBundle
    {
        public IReadOnlyList<SchemaFile> SchemaFiles;
    }

    public class Package
    {
        public SourceReference SourceReference;
        public string Name;
    }

    public class Import
    {
        public SourceReference SourceReference;
        public string Path;
    }

    public class SchemaFile
    {
        public string CanonicalPath;
        public Package Package;
        public IReadOnlyList<Import> Imports;
        public IReadOnlyList<EnumDefinition> Enums;
        public IReadOnlyList<TypeDefinition> Types;
        public IReadOnlyList<ComponentDefinition> Components;
    }
}
