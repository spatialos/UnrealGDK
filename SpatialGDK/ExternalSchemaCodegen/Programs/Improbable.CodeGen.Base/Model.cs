using System;
using System.Collections.Generic;
using System.Diagnostics;
using Newtonsoft.Json;

namespace Improbable.CodeGen.Base
{
    [DebuggerDisplay("{" + nameof(QualifiedName) + "}")]
    public class Identifier : IEquatable<Identifier>
    {
        // These properties are explicitly tagged with [JsonProperty] so that JSON.Net can deserialize into them as readonly fields.
        [JsonProperty] public readonly string Name;
        [JsonProperty] public readonly IReadOnlyList<string> Path;
        [JsonProperty] public readonly string QualifiedName;

        public bool Equals(Identifier other)
        {
            if (ReferenceEquals(null, other))
            {
                return false;
            }

            if (ReferenceEquals(this, other))
            {
                return true;
            }

            return string.Equals(QualifiedName, other.QualifiedName);
        }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj))
            {
                return false;
            }

            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            if (obj.GetType() != GetType())
            {
                return false;
            }

            return Equals((Identifier) obj);
        }

        public override int GetHashCode()
        {
            var hashCode = QualifiedName != null ? QualifiedName.GetHashCode() : 0;
            return hashCode;
        }
    }

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
        Bytes = 16
    }

    public class TypeReference
    {
        public string QualifiedName;
    }

    public class EnumReference
    {
        public string QualifiedName;
    }

    public class EnumValueReference
    {
        public string QualifiedName;
    }

    public class FieldReference
    {
        public string QualifiedName;
    }

    public enum ValueType
    {
        Enum,
        Primitive,
        Type
    }

    public class ValueTypeReference
    {
        public EnumReference Enum;
        public PrimitiveType Primitive;
        public TypeReference Type;

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

                throw new InvalidOperationException("ValueTypeReference doesn't have any type set.");
            }
        }
    }

    public class Value
    {
        public bool BoolValue;
        public byte[] BytesValue;
        public double DoubleValue;
        public long EntityIdValue;
        public float FloatValue;
        public int Int32Value;
        public long Int64Value;
        public ListValueHolder ListValue;
        public MapValueHolder MapValue;
        public OptionValueHolder OptionValue;
        public SchemaEnumValue SchemaEnumValue;
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
        public EnumReference Enum;
        public EnumValueReference EnumValue;

        public string Name;
        public uint Value;
    }

    public class TypeValue
    {
        public IReadOnlyList<FieldValue> Fields;

        public TypeReference Type;

        public class FieldValue
        {
            public FieldReference Field;
            public string Name;
            public uint Number;
            public Value Value;
        }
    }

    public class Annotation
    {
        public TypeValue TypeValue;
    }

    public class EnumValueDefinition
    {
        public IReadOnlyList<Annotation> Annotations;
        public Identifier Identifier;

        public uint Value;
    }

    [DebuggerDisplay("{" + nameof(Identifier) + "}")]
    public class EnumDefinition
    {
        public IReadOnlyList<Annotation> Annotations;
        public Identifier Identifier;
        public IReadOnlyList<EnumValueDefinition> ValueDefinitions;
    }

    public enum FieldType
    {
        Option,
        List,
        Map,
        Singular
    }

    [DebuggerDisplay("{" + nameof(Identifier) + "}" + " {" + nameof(FieldId) + "}")]
    public class FieldDefinition
    {
        public IReadOnlyList<Annotation> Annotations;

        public uint FieldId;

        public Identifier Identifier;
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
            public ValueTypeReference Type;
        }

        public class OptionTypeRef
        {
            public ValueTypeReference InnerType;
        }

        public class ListTypeRef
        {
            public ValueTypeReference InnerType;
        }

        public class MapTypeRef
        {
            public ValueTypeReference KeyType;
            public ValueTypeReference ValueType;
        }
    }

    [DebuggerDisplay("{" + nameof(Identifier) + "}")]
    public class TypeDefinition
    {
        public IReadOnlyList<Annotation> Annotations;
        public IReadOnlyList<FieldDefinition> FieldDefinitions;
        public Identifier Identifier;
    }

    [DebuggerDisplay("{" + nameof(Identifier) + "} {" + nameof(ComponentId) + "}")]
    public class ComponentDefinition
    {
        public IReadOnlyList<Annotation> Annotations;
        public IReadOnlyList<CommandDefinition> CommandDefinitions;
        public uint ComponentId;

        public TypeReference DataDefinition;
        public IReadOnlyList<EventDefinition> EventDefinitions;

        public IReadOnlyList<FieldDefinition> FieldDefinitions;

        public Identifier Identifier;

        [DebuggerDisplay("{" + nameof(Identifier) + "}")]
        public class EventDefinition
        {
            public IReadOnlyList<Annotation> Annotations;
            public uint EventIndex;
            public Identifier Identifier;
            public ValueTypeReference Type;
        }

        [DebuggerDisplay("{" + nameof(Identifier) + "}" + " {" + nameof(CommandIndex) + "}")]
        public class CommandDefinition
        {
            public IReadOnlyList<Annotation> Annotations;
            public uint CommandIndex;

            public Identifier Identifier;
            public ValueTypeReference RequestType;
            public ValueTypeReference ResponseType;
        }
    }

    public class SchemaBundleV1
    {
        public IReadOnlyList<ComponentDefinition> ComponentDefinitions;

        public IReadOnlyList<EnumDefinition> EnumDefinitions;
        public IReadOnlyList<TypeDefinition> TypeDefinitions;
    }

    public class SourceReference
    {
        public uint Column;
        public string FilePath;
        public uint Line;
    }

    public class SchemaSourceMapV1
    {
        public Dictionary<string, SourceReference> SourceReferences;
    }

    public class SchemaBundle
    {
        public SchemaSourceMapV1 SourceMapV1;
        public SchemaBundleV1 V1;
    }
}   
