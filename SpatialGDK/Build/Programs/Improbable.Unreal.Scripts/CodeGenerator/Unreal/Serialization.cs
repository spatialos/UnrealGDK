using Improbable.CodeGen.Base;
using System;
using ValueType = Improbable.CodeGen.Base.ValueType;

namespace Improbable.CodeGen.Unreal
{
    public static class Serialization
    {
        public static string GetFieldSerialization(FieldDefinition field, string schemaObjectName, string targetObjectName, TypeDescription parentType, Bundle bundle)
        {
            switch (field.TypeSelector)
            {
                case FieldType.Singular:
                    return GetValueTypeSerialization(field.SingularType.Type, schemaObjectName, targetObjectName, field.FieldId.ToString());
                case FieldType.Option:
                    return GetOptionTypeSerialization(field.OptionType, schemaObjectName, targetObjectName, field.FieldId.ToString(), bundle);
                case FieldType.List:
                    return GetListTypeSerialization(field.ListType, schemaObjectName, targetObjectName, field.FieldId.ToString(), parentType, bundle);
                case FieldType.Map:
                    return GetMapTypeSerialization(field.MapType, schemaObjectName, targetObjectName, field.FieldId.ToString(), parentType, bundle);
                default:
                    throw new InvalidOperationException("Trying to serialize invalid FieldDefinition");
            }
        }

        public static string GetFieldDeserialization(FieldDefinition field, string schemaObjectName, string targetObjectName, TypeDescription parentType, Bundle bundle, bool wrapInBlock = false, bool targetIsOption = false)
        {
            var fieldName = Text.SnakeCaseToPascalCase(field.Name);
            switch (field.TypeSelector)
            {
                case FieldType.Singular:
                    return $"{targetObjectName} = {GetValueTypeDeserialization(field.SingularType.Type, schemaObjectName, field.FieldId.ToString(), parentType)};";
                case FieldType.Option:
                    return $"{targetObjectName} = {Types.CollectionTypesToQualifiedTypes[Types.Collection.Option]}<{Types.GetTypeDisplayName(field.OptionType.InnerType, bundle, parentType)}>({GetValueTypeDeserialization(field.OptionType.InnerType, schemaObjectName, field.FieldId.ToString(), parentType)});";
                case FieldType.List:
                    return GetListTypeDeserialization(field.ListType, schemaObjectName, targetObjectName, fieldName, field.FieldId.ToString(), parentType, bundle, wrapInBlock, targetIsOption);
                case FieldType.Map:
                    return GetMapTypeDeserialization(field.MapType, schemaObjectName, targetObjectName, fieldName, field.FieldId.ToString(), parentType, bundle, wrapInBlock, targetIsOption);
                default:
                    throw new InvalidOperationException("Trying to serialize invalid FieldDefinition");
            }
        }

        public static string GetEventDeserialization(ComponentDefinition.EventDefinition _event, string schemaObjectName, string targetObjectName)
        {
            return $@"for (uint32 i = 0; i < Schema_GetObjectCount({schemaObjectName}, {_event.EventIndex}); ++i)
{{
{Text.Indent(1, $"{targetObjectName}.Add{Text.SnakeCaseToPascalCase(_event.Name)}({Types.GetTypeDisplayName(_event.Type)}::Deserialize(Schema_IndexObject({schemaObjectName}, {_event.EventIndex}, i)));")}
}}";
        }

        public static string GetFieldClearingCheck(FieldDefinition field)
        {
            var fieldName = Text.SnakeCaseToPascalCase(field.Name);
            switch (field.TypeSelector)
            {
                case FieldType.Option:
                    return $"!_{Text.SnakeCaseToPascalCase(fieldName)}.GetValue().IsSet()";
                case FieldType.List:
                case FieldType.Map:
                    return $"_{Text.SnakeCaseToPascalCase(fieldName)}.GetValue().Num() == 0";
                case FieldType.Singular:
                    throw new InvalidOperationException("Trying to check if singular type should be clear should never happen");
                default:
                    throw new InvalidOperationException("Trying to get field clearing check for invalid FieldDefinition");
            }
        }

        public static string GetFieldTypeCount(FieldDefinition field, string schemaObjectName)
        {
            switch (field.TypeSelector)
            {
                case FieldType.Option:
                    return GetValueTypeCount(field.OptionType.InnerType, schemaObjectName, field.FieldId.ToString());
                case FieldType.List:
                    return GetValueTypeCount(field.ListType.InnerType, schemaObjectName, field.FieldId.ToString());
                case FieldType.Map:
                    return $"Schema_GetObjectCount({schemaObjectName}, {field.FieldId})";
                case FieldType.Singular:
                    return GetValueTypeCount(field.SingularType.Type, schemaObjectName, field.FieldId.ToString());
                default:
                    throw new InvalidOperationException("Trying to get schema object count for invalid FieldDefinition");
            }
        }

        private static string GetValueTypeSerialization(TypeReference value, string schemaObjectName, string targetObjectName, string fieldId)
        {
            switch (value.ValueTypeSelector)
            {
                case ValueType.Primitive:
                    return GetPrimitiveSerialization(value.Primitive, schemaObjectName, targetObjectName, fieldId);
                case ValueType.Enum:
                    return $"Schema_AddEnum({schemaObjectName}, {fieldId}, static_cast<uint32>({targetObjectName}));";
                case ValueType.Type:
                    return $"{targetObjectName}.Serialize(Schema_AddObject({schemaObjectName}, {fieldId}));";
                default:
                    throw new InvalidOperationException("Trying to serialize invalid TypeReference");
            }
        }

        private static string GetOptionTypeSerialization(FieldDefinition.OptionTypeRef optionType, string schemaObjectName, string fieldName, string fieldId, Bundle bundle)
        {
            return $@"if ({fieldName})
{{
{Text.Indent(1, GetValueTypeSerialization(optionType.InnerType, schemaObjectName, $"(*{fieldName})", fieldId))}
}}";
        }

        private static string GetMapTypeSerialization(FieldDefinition.MapTypeRef mapType, string schemaObjectName, string fieldName, string fieldId, TypeDescription parentType, Bundle bundle)
        {
            return $@"for (const TPair<{Types.GetTypeDisplayName(mapType.KeyType, bundle, parentType)}, {Types.GetTypeDisplayName(mapType.ValueType, bundle, parentType)}>& Pair : {fieldName})
{{
{Text.Indent(1, $@"Schema_Object* PairObj = Schema_AddObject({schemaObjectName}, {fieldId});
{GetValueTypeSerialization(mapType.KeyType, "PairObj", "Pair.Key", "SCHEMA_MAP_KEY_FIELD_ID")}
{GetValueTypeSerialization(mapType.ValueType, "PairObj", "Pair.Value", "SCHEMA_MAP_VALUE_FIELD_ID")}")}
}}";
        }

        private static string GetListTypeSerialization(FieldDefinition.ListTypeRef listType, string schemaObjectName, string fieldName, string fieldId, TypeDescription parentType, Bundle bundle)
        {
            return $@"for (const {Types.GetTypeDisplayName(listType.InnerType, bundle, parentType)}& Element : {fieldName})
{{
{Text.Indent(1, GetValueTypeSerialization(listType.InnerType, schemaObjectName, "Element", fieldId))}
}}";
        }

        private static string GetPrimitiveSerialization(PrimitiveType primitive, string schemaObjectName, string fieldName, string fieldId)
        {
            switch (primitive)
            {
                case PrimitiveType.Int32:
                    return $"Schema_AddInt32({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Int64:
                    return $"Schema_AddInt64({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Uint32:
                    return $"Schema_AddUint32({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Uint64:
                    return $"Schema_AddUint64({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Sint32:
                    return $"Schema_AddSint32({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Sint64:
                    return $"Schema_AddSint64({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Fixed32:
                    return $"Schema_AddFixed32({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Fixed64:
                    return $"Schema_AddFixed64({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Sfixed32:
                    return $"Schema_AddSfixed32({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Sfixed64:
                    return $"Schema_AddSfixed64({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Bool:
                    return $"Schema_AddBool({schemaObjectName}, {fieldId}, static_cast<uint8>({fieldName}));";
                case PrimitiveType.Float:
                    return $"Schema_AddFloat({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Double:
                    return $"Schema_AddDouble({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.String:
                    return $@"::improbable::utils::AddString({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.EntityId:
                    return $"Schema_AddEntityId({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Bytes:
                    return $@"::improbable::utils::AddBytes({schemaObjectName}, {fieldId}, {fieldName});";
                case PrimitiveType.Invalid:
                default:
                    throw new InvalidOperationException("Trying to serialize invalid PrimitiveType");
            }
        }

        private static string GetValueTypeDeserialization(TypeReference type, string schemaObjectName, string fieldId, TypeDescription parentType)
        {
            switch (type.ValueTypeSelector)
            {
                case ValueType.Primitive:
                    return GetPrimitiveDeserialization(type.Primitive, schemaObjectName, fieldId);
                case ValueType.Type:
                    return $"{Types.GetTypeDisplayName(type.Type, Types.IsTypeBeingUsedInTheContextWhereItIsDefined(type.Type, parentType))}::Deserialize(Schema_GetObject({schemaObjectName}, {fieldId}))";
                case ValueType.Enum:
                    return $"static_cast<{Types.GetTypeDisplayName(type.Enum, Types.IsTypeBeingUsedInTheContextWhereItIsDefined(type.Enum, parentType))}>(Schema_GetEnum({ schemaObjectName}, { fieldId}))";
                default:
                    throw new InvalidOperationException("Trying to deserialize invalid TypeReference");
            }
        }

        private static string GetListTypeDeserialization(FieldDefinition.ListTypeRef listType, string schemaObjectName, string targetObjectName, string fieldName, string fieldId, TypeDescription parentType,
            Bundle bundle, bool wrapInBlock = false, bool targetIsOption = false)
        {
            var listInnerTypeName = Types.GetTypeDisplayName(listType.InnerType, bundle, parentType);
            var listName = $"{Text.SnakeCaseToPascalCase(fieldName)}List";

            var deserializationText = "";
            switch (listType.InnerType.ValueTypeSelector)
            {
                case ValueType.Primitive:
                    deserializationText = GetPrimitiveListDeserialization(listType.InnerType.Primitive, schemaObjectName, targetObjectName, fieldName, fieldId, bundle, targetIsOption);
                    break;
                case ValueType.Type:
                    deserializationText = $@"auto ListLength = Schema_GetObjectCount({schemaObjectName}, {fieldId});
{Types.CollectionTypesToQualifiedTypes[Types.Collection.List]}<{listInnerTypeName}> {listName};
{listName}.SetNum(ListLength);
for (uint32 i = 0; i < ListLength; ++i)
{{
{Text.Indent(1, $"{listName}[i] = {listInnerTypeName}::Deserialize(Schema_IndexObject({schemaObjectName}, {fieldId}, i));")}
}}
{targetObjectName} = {listName};";
                    break;
                case ValueType.Enum:
                    deserializationText = $@"auto ListLength = Schema_GetEnumCount({schemaObjectName}, {fieldId});
{Types.CollectionTypesToQualifiedTypes[Types.Collection.List]}<{listInnerTypeName}> {listName};
{listName}.SetNum(ListLength);
for (uint32 i = 0; i < ListLength; ++i)
{{
{Text.Indent(1, $"{listName}[i] = static_cast<{listInnerTypeName}>(Schema_IndexEnum({schemaObjectName}, {fieldId}, i));")}
}}
{targetObjectName} = {listName};";
                    break;
                default:
                    throw new InvalidOperationException("Trying to deserialize invalid TypeReference");
            }

            return wrapInBlock ? $"{{{Environment.NewLine}{Text.Indent(1, deserializationText)}{Environment.NewLine}}}" : deserializationText;
        }

        private static string GetMapTypeDeserialization(FieldDefinition.MapTypeRef mapType, string schemaObjectName, string targetObjectName, string fieldName, string fieldId,
            TypeDescription parentType, Bundle bundle, bool wrapInBlock = false, bool targetIsOption = false)
        {
            var keyTypeName = Types.GetTypeDisplayName(mapType.KeyType, bundle, parentType);
            var valueTypeName = Types.GetTypeDisplayName(mapType.ValueType, bundle, parentType);
            var mapName = $"{fieldName}Map";
            var kvPairName = "KvPair";

            var deserializationText = $@"{{
{Text.Indent(1, $@"{targetObjectName} = {Types.CollectionTypesToQualifiedTypes[Types.Collection.Map]}<{keyTypeName}, {valueTypeName}>();
auto MapEntryCount = Schema_GetObjectCount({schemaObjectName}, {fieldId});
for (uint32 i = 0; i < MapEntryCount; ++i)
{{
{Text.Indent(1, $@"Schema_Object* {kvPairName} = Schema_IndexObject({schemaObjectName}, {fieldId}, i);
auto Key = {GetValueTypeDeserialization(mapType.KeyType, kvPairName, "SCHEMA_MAP_KEY_FIELD_ID", parentType)};
auto Value = {GetValueTypeDeserialization(mapType.ValueType, kvPairName, "SCHEMA_MAP_VALUE_FIELD_ID", parentType)};
{(targetIsOption ? $"(*{targetObjectName})" : targetObjectName)}[std::move(Key)] = std::move(Value);")}
}}")}
}}";

            return wrapInBlock ? $"{{{Environment.NewLine}{Text.Indent(1, deserializationText)}{Environment.NewLine}}}" : deserializationText;
        }

        private static string GetPrimitiveListDeserialization(PrimitiveType primitive, string schemaObjectName, string targetObjectName, string fieldName, string fieldId, Bundle bundle, bool targetIsOption = false)
        {
            targetObjectName = targetIsOption ? $"(*{targetObjectName})" : targetObjectName;

            if (primitive == PrimitiveType.Bytes)
            {
                return $"{targetObjectName} = ::improbable::utils::GetBytesList({schemaObjectName}, {fieldId});";
            }
            else if (primitive == PrimitiveType.String)
            {
                return $"{targetObjectName} = ::improbable::utils::GetStringList({schemaObjectName}, {fieldId});";
            }

            var listInnerType = Types.SchemaToCppTypes[primitive];
            var listCopyFunction = "";
            switch (primitive)
            {
                case PrimitiveType.Int32:
                    listCopyFunction = $"Schema_GetInt32List({ schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Int64:
                    listCopyFunction = $"Schema_GetInt64List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Uint32:
                    listCopyFunction = $"Schema_GetUint32List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Uint64:
                    listCopyFunction = $"Schema_GetUint64List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Sint32:
                    listCopyFunction = $"Schema_GetSint32List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Sint64:
                    listCopyFunction = $"Schema_GetSint64List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Fixed32:
                    listCopyFunction = $"Schema_GetFixed32List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Fixed64:
                    listCopyFunction = $"Schema_GetFixed64List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Sfixed32:
                    listCopyFunction = $"Schema_GetSfixed32List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Sfixed64:
                    listCopyFunction = $"Schema_GetSfixed64List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Bool:
                    listCopyFunction = $"Schema_GetBoolList({schemaObjectName}, {fieldId}, (uint8*) {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Float:
                    listCopyFunction = $"Schema_GetFloatList({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Double:
                    listCopyFunction = $"Schema_GetDoubleList({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.EntityId:
                    listCopyFunction = $"Schema_GetEntityIdList({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());";
                    break;
                case PrimitiveType.Invalid:
                default:
                    throw new InvalidOperationException("Trying to serialize invalid PrimitiveType");
            }

            return $@"uint32 {fieldName}Length = {GetPrimitiveCount(primitive, schemaObjectName, fieldId)};
{Types.GetListInitialisation(targetObjectName, listInnerType, $"{fieldName}Length")}
{listCopyFunction}";
        }

        private static string GetPrimitiveDeserialization(PrimitiveType primitive, string schemaObjectName, string fieldId)
        {
            switch (primitive)
            {
                case PrimitiveType.Int32:
                    return $"Schema_GetInt32({ schemaObjectName}, { fieldId})";
                case PrimitiveType.Int64:
                    return $"Schema_GetInt64({schemaObjectName}, {fieldId})";
                case PrimitiveType.Uint32:
                    return $"Schema_GetUint32({schemaObjectName}, {fieldId})";
                case PrimitiveType.Uint64:
                    return $"Schema_GetUint64({schemaObjectName}, {fieldId})";
                case PrimitiveType.Sint32:
                    return $"Schema_GetSint32({schemaObjectName}, {fieldId})";
                case PrimitiveType.Sint64:
                    return $"Schema_GetSint64({schemaObjectName}, {fieldId})";
                case PrimitiveType.Fixed32:
                    return $"Schema_GetFixed32({schemaObjectName}, {fieldId})";
                case PrimitiveType.Fixed64:
                    return $"Schema_GetFixed32({schemaObjectName}, {fieldId})";
                case PrimitiveType.Sfixed32:
                    return $"Schema_GetSfixed32({schemaObjectName}, {fieldId})";
                case PrimitiveType.Sfixed64:
                    return $"Schema_GetSfixed64({schemaObjectName}, {fieldId})";
                case PrimitiveType.Bool:
                    return $"Schema_GetBool({schemaObjectName}, {fieldId})";
                case PrimitiveType.Float:
                    return $"Schema_GetFloat({schemaObjectName}, {fieldId})";
                case PrimitiveType.Double:
                    return $"Schema_GetDouble({schemaObjectName}, {fieldId})";
                case PrimitiveType.EntityId:
                    return $"Schema_GetEntityId({schemaObjectName}, {fieldId})";
                case PrimitiveType.Bytes:
                    return $"::improbable::utils::GetBytes({schemaObjectName}, {fieldId})";
                case PrimitiveType.String:
                    return $"::improbable::utils::GetString({schemaObjectName}, {fieldId})";
                case PrimitiveType.Invalid:
                default:
                    throw new InvalidOperationException("Trying to serialize invalid PrimitiveType");
            }
        }

        private static string GetValueTypeCount(TypeReference type, string schemaObjectName, string fieldId)
        {
            switch (type.ValueTypeSelector)
            {
                case ValueType.Primitive:
                    return GetPrimitiveCount(type.Primitive, schemaObjectName, fieldId);
                case ValueType.Type:
                    return $"Schema_GetObjectCount({ schemaObjectName}, { fieldId})";
                case ValueType.Enum:
                    return $"Schema_GetEnumCount({ schemaObjectName}, { fieldId})";
                default:
                    throw new InvalidOperationException("Trying to deserialize invalid TypeReference");
            }
        }

        private static string GetPrimitiveCount(PrimitiveType primitive, string schemaObjectName, string fieldId)
        {
            switch (primitive)
            {
                case PrimitiveType.Int32:
                    return $"Schema_GetInt32Count({ schemaObjectName}, { fieldId})";
                case PrimitiveType.Int64:
                    return $"Schema_GetInt64Count({schemaObjectName}, {fieldId})";
                case PrimitiveType.Uint32:
                    return $"Schema_GetUint32Count({schemaObjectName}, {fieldId})";
                case PrimitiveType.Uint64:
                    return $"Schema_GetUint64Count({schemaObjectName}, {fieldId})";
                case PrimitiveType.Sint32:
                    return $"Schema_GetSint32Count({schemaObjectName}, {fieldId})";
                case PrimitiveType.Sint64:
                    return $"Schema_GetSint64Count({schemaObjectName}, {fieldId})";
                case PrimitiveType.Fixed32:
                    return $"Schema_GetFixed32Count({schemaObjectName}, {fieldId})";
                case PrimitiveType.Fixed64:
                    return $"Schema_GetFixed32Count({schemaObjectName}, {fieldId})";
                case PrimitiveType.Sfixed32:
                    return $"Schema_GetSfixed32Count({schemaObjectName}, {fieldId})";
                case PrimitiveType.Sfixed64:
                    return $"Schema_GetSfixed64Count({schemaObjectName}, {fieldId})";
                case PrimitiveType.Bool:
                    return $"Schema_GetBoolCount({schemaObjectName}, {fieldId})";
                case PrimitiveType.Float:
                    return $"Schema_GetFloatCount({schemaObjectName}, {fieldId})";
                case PrimitiveType.Double:
                    return $"Schema_GetDoubleCount({schemaObjectName}, {fieldId})";
                case PrimitiveType.String:
                    return $"Schema_GetBytesCount({schemaObjectName}, {fieldId})";
                case PrimitiveType.EntityId:
                    return $"Schema_GetEntityIdCount({schemaObjectName}, {fieldId})";
                case PrimitiveType.Bytes:
                    return $"Schema_GetBytesCount({schemaObjectName}, {fieldId})";
                case PrimitiveType.Invalid:
                default:
                    throw new InvalidOperationException("Trying to serialize invalid PrimitiveType");
            }
        }
    }
}
