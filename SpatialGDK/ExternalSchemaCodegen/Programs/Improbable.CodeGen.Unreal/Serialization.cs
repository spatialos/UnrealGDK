using Improbable.CodeGen.Base;
using System;
using System.Text;
using ValueType = Improbable.CodeGen.Base.ValueType;

namespace Improbable.CodeGen.Unreal
{
    public static class Serialization
    {
        public static string GetFieldSerialization(FieldDefinition field, string schemaObjectName, string targetObjectName,  Bundle bundle)
        {
            switch (field.TypeSelector)
            {
                case FieldType.Singular:
                    return GetValueTypeSerialization(field.SingularType.Type, schemaObjectName, targetObjectName, field.FieldId.ToString());
                case FieldType.Option:
                    return GetOptionTypeSerialization(field.OptionType, schemaObjectName, targetObjectName, field.FieldId.ToString(), bundle);
                case FieldType.List:
                    return GetListTypeSerialization(field.ListType, schemaObjectName, targetObjectName, field.FieldId.ToString(), bundle);
                case FieldType.Map:
                    return GetMapTypeSerialization(field.MapType, schemaObjectName, targetObjectName, field.FieldId.ToString(), bundle);
                default:
                    throw new InvalidOperationException("Trying to serialize invalid FieldDefinition");
            }
        }

        public static string GetFieldDeserialization(FieldDefinition field, string schemaObjectName, string targetObjectName, Bundle bundle, bool wrapInBlock = false, bool targetIsOption = false)
        {
            var fieldName = Text.SnakeCaseToPascalCase(field.Identifier.Name);
            switch (field.TypeSelector)
            {
                case FieldType.Singular:
                    return $"{targetObjectName} = {GetValueTypeDeserialization(field.SingularType.Type, schemaObjectName, field.FieldId.ToString())};";
                case FieldType.Option:
                    return GetOptionTypeDeserialization(field.OptionType, schemaObjectName, targetObjectName, fieldName, field.FieldId.ToString(), bundle, targetIsOption);
                case FieldType.List:
                    return GetListTypeDeserialization(field.ListType, schemaObjectName, targetObjectName, fieldName, field.FieldId.ToString(), bundle, wrapInBlock, targetIsOption);
                case FieldType.Map:
                    return GetMapTypeDeserialization(field.MapType, schemaObjectName, targetObjectName, fieldName, field.FieldId.ToString(), bundle, wrapInBlock, targetIsOption);
                default:
                    throw new InvalidOperationException("Trying to serialize invalid FieldDefinition");
            }
        }

        public static string GetEventDeserialization(ComponentDefinition.EventDefinition _event, string schemaObjectName, string targetObjectName)
        {
            var builder = new StringBuilder();
            builder.AppendLine($"for (uint32 i = 0; i < Schema_GetObjectCount({schemaObjectName}, {_event.EventIndex}); ++i)");
            builder.AppendLine("{");
            builder.AppendLine(Text.Indent(1, $"{targetObjectName}.Add{Text.SnakeCaseToPascalCase(_event.Identifier.Name)}({Types.GetTypeDisplayName(_event.Type.Type.QualifiedName)}::Deserialize(Schema_IndexObject({schemaObjectName}, {_event.EventIndex}, i)));"));
            builder.AppendLine("}");
            return builder.ToString();
        }

        public static string GetFieldClearingCheck(FieldDefinition field)
        {
            var fieldName = Text.SnakeCaseToPascalCase(field.Identifier.Name);
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

        private static string GetValueTypeSerialization(ValueTypeReference value, string schemaObjectName, string targetObjectName, string fieldId)
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
                    throw new InvalidOperationException("Trying to serialize invalid ValueTypeReference");
            }
        }

        private static string GetOptionTypeSerialization(FieldDefinition.OptionTypeRef optionType, string schemaObjectName, string fieldName, string fieldId, Bundle bundle)
        {
            var builder = new StringBuilder();
            builder.AppendLine($"if ({fieldName})");
            builder.AppendLine("{");
            builder.AppendLine(Text.Indent(1, GetValueTypeSerialization(optionType.InnerType, schemaObjectName, $"(*{fieldName})", fieldId)));
            builder.AppendLine("}");
            return builder.ToString();
        }

        private static string GetMapTypeSerialization(FieldDefinition.MapTypeRef mapType, string schemaObjectName, string fieldName, string fieldId, Bundle bundle)
        {
            var builder = new StringBuilder();
            builder.AppendLine($@"for (const TPair<{Types.GetTypeDisplayName(mapType.KeyType, bundle)}, {Types.GetTypeDisplayName(mapType.ValueType, bundle)}>& Pair : {fieldName})");
            builder.AppendLine("{");
            builder.AppendLine(Text.Indent(1, $"Schema_Object* PairObj = Schema_AddObject({schemaObjectName}, {fieldId});"));
            builder.AppendLine(Text.Indent(1, GetValueTypeSerialization(mapType.KeyType, "PairObj", "Pair.Key", "SCHEMA_MAP_KEY_FIELD_ID")));
            builder.AppendLine(Text.Indent(1, GetValueTypeSerialization(mapType.ValueType, "PairObj", "Pair.Value", "SCHEMA_MAP_VALUE_FIELD_ID")));
            builder.AppendLine("}");
            return builder.ToString();
        }

        private static string GetListTypeSerialization(FieldDefinition.ListTypeRef listType, string schemaObjectName, string fieldName, string fieldId, Bundle bundle)
        {
            var builder = new StringBuilder();
            builder.AppendLine($@"for (const {Types.GetTypeDisplayName(listType.InnerType, bundle)}& Element : {fieldName})");
            builder.AppendLine("{");
            builder.AppendLine(Text.Indent(1, GetValueTypeSerialization(listType.InnerType, schemaObjectName, "Element", fieldId)));
            builder.AppendLine("}");
            return builder.ToString();
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

        private static string GetValueTypeDeserialization(ValueTypeReference type, string schemaObjectName, string fieldId)
        {
            switch (type.ValueTypeSelector)
            {
                case ValueType.Primitive:
                    return GetPrimitiveDeserialization(type.Primitive, schemaObjectName, fieldId);
                case ValueType.Type:
                    return $"{Types.GetTypeDisplayName(type.Type.QualifiedName)}::Deserialize(Schema_GetObject({schemaObjectName}, {fieldId}))";
                case ValueType.Enum:
                    return $"static_cast<{Types.GetTypeDisplayName(type.Enum.QualifiedName)}>(Schema_GetEnum({ schemaObjectName}, { fieldId}))";
                default:
                    throw new InvalidOperationException("Trying to deserialize invalid ValueTypeReference");
            }
        }

        private static string GetOptionTypeDeserialization(FieldDefinition.OptionTypeRef optionType, string schemaObjectName, string targetObjectName, string fieldName, string fieldId, Bundle bundle, bool targetIsOption = false)
        {
            return $"{targetObjectName} = {Types.CollectionTypesToQualifiedTypes[Types.Collection.Option]}<{Types.GetTypeDisplayName(optionType.InnerType, bundle)}>({GetValueTypeDeserialization(optionType.InnerType, schemaObjectName, fieldId)});";
        }

        private static string GetListTypeDeserialization(FieldDefinition.ListTypeRef listType, string schemaObjectName, string targetObjectName, string fieldName, string fieldId, Bundle bundle, bool wrapInBlock = false, bool targetIsOption = false)
        {
            var listInnerTypeName = Types.GetTypeDisplayName(listType.InnerType, bundle);
            var listName = $"{Text.SnakeCaseToPascalCase(fieldName)}List";
            var builder = new StringBuilder();

            switch (listType.InnerType.ValueTypeSelector)
            {
                case ValueType.Primitive:
                    builder.AppendLine(GetPrimitiveListDeserialization(listType.InnerType.Primitive, schemaObjectName, targetObjectName, fieldName, fieldId, bundle, targetIsOption));
                    break;
                case ValueType.Type:
                    builder.AppendLine($@"auto ListLength = Schema_GetObjectCount({schemaObjectName}, {fieldId});");
                    builder.AppendLine($@"{Types.CollectionTypesToQualifiedTypes[Types.Collection.List]}<{listInnerTypeName}> {listName};");
                    builder.AppendLine($@"{listName}.SetNum(ListLength);");
                    builder.AppendLine($@"for (uint32 i = 0; i < ListLength; ++i)");
                    builder.AppendLine($@"{{");
                    builder.AppendLine(Text.Indent(1, $"{listName}[i] = {listInnerTypeName}::Deserialize(Schema_IndexObject({schemaObjectName}, {fieldId}, i));"));
                    builder.AppendLine($@"}}");
                    builder.AppendLine($@"{targetObjectName} = {listName};");
                    break;
                case ValueType.Enum:
                    builder.AppendLine($@"auto ListLength = Schema_GetEnumCount({schemaObjectName}, {fieldId});");
                    builder.AppendLine($@"{Types.CollectionTypesToQualifiedTypes[Types.Collection.List]}<{listInnerTypeName}> {listName};");
                    builder.AppendLine($@"{listName}.SetNum(ListLength);");
                    builder.AppendLine($@"for (uint32 i = 0; i < ListLength; ++i)");
                    builder.AppendLine($@"{{");
                    builder.AppendLine(Text.Indent(1, $"{listName}[i] = static_cast<{listInnerTypeName}>(Schema_IndexEnum({schemaObjectName}, {fieldId}, i));"));
                    builder.AppendLine($@"}}");
                    builder.AppendLine($@"{targetObjectName} = {listName};");
                    break;
                default:
                    throw new InvalidOperationException("Trying to deserialize invalid ValueTypeReference");
            }

            if (wrapInBlock)
            {
                var blockWrapping = new StringBuilder();
                blockWrapping.AppendLine("{");
                blockWrapping.AppendLine(Text.Indent(1, builder.ToString()));
                blockWrapping.AppendLine("}");
                return blockWrapping.ToString();
            }

            return builder.ToString();
        }

        private static string GetMapTypeDeserialization(FieldDefinition.MapTypeRef mapType, string schemaObjectName, string targetObjectName, string fieldName, string fieldId, Bundle bundle, bool wrapInBlock = false, bool targetIsOption = false)
        {
            var keyTypeName = Types.GetTypeDisplayName(mapType.KeyType, bundle);
            var valueTypeName = Types.GetTypeDisplayName(mapType.ValueType, bundle);
            var mapName = $"{ fieldName }_map";
            var kvPairName = "kvpair";
            var builder = new StringBuilder();

            builder.AppendLine($"{targetObjectName} = {Types.CollectionTypesToQualifiedTypes[Types.Collection.Map]}<{keyTypeName}, {valueTypeName}>();");
            builder.AppendLine($"auto MapEntryCount = Schema_GetObjectCount({schemaObjectName}, {fieldId});");
            builder.AppendLine($"for (uint32 i = 0; i < MapEntryCount; ++i)");
            builder.AppendLine("{");
            builder.AppendLine(Text.Indent(1, $@"Schema_Object* {kvPairName} = Schema_IndexObject({schemaObjectName}, {fieldId}, i);"));
            builder.AppendLine(Text.Indent(1, $@"auto Key = {GetValueTypeDeserialization(mapType.KeyType, kvPairName, "SCHEMA_MAP_KEY_FIELD_ID")};"));
            builder.AppendLine(Text.Indent(1, $@"auto Value = {GetValueTypeDeserialization(mapType.ValueType, kvPairName, "SCHEMA_MAP_VALUE_FIELD_ID")};"));
            builder.AppendLine(Text.Indent(1, $@"{(targetIsOption ? $"(*{targetObjectName})" : targetObjectName)}[std::move(Key)] = std::move(Value);"));
            builder.AppendLine("}");

            if (wrapInBlock)
            {
                var blockWrapping = new StringBuilder();
                blockWrapping.AppendLine("{");
                blockWrapping.AppendLine(Text.Indent(1, builder.ToString()));
                blockWrapping.AppendLine("}");
                return blockWrapping.ToString().TrimEnd();
            }

            return builder.ToString();
        }

        private static string GetPrimitiveListDeserialization(PrimitiveType primitive, string schemaObjectName, string targetObjectName, string fieldName, string fieldId, Bundle bundle, bool targetIsOption = false)
        {
            if (primitive == PrimitiveType.Bytes)
            {
                return $"{targetObjectName} = ::improbable::utils::GetBytesList({schemaObjectName}, {fieldId});";
            }
            else if (primitive == PrimitiveType.String)
            {
                return $"{targetObjectName} = ::improbable::utils::GetStringList({schemaObjectName}, {fieldId});";
            }

            var listInnerType = Types.SchemaToCppTypes[primitive];
            var builder = new StringBuilder();

            builder.AppendLine($"uint32 {fieldName}Length = {GetPrimitiveCount(primitive, schemaObjectName, fieldId)};");
            builder.AppendLine(Types.GetListInitialisation(targetObjectName, listInnerType, $"{fieldName}Length", targetIsOption));

            targetObjectName = targetIsOption ? $"(*{targetObjectName})" : targetObjectName;
            switch (primitive)
            {
                case PrimitiveType.Int32:
                    builder.AppendLine($"Schema_GetInt32List({ schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Int64:
                    builder.AppendLine($"Schema_GetInt64List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Uint32:
                    builder.AppendLine($"Schema_GetUint32List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Uint64:
                    builder.AppendLine($"Schema_GetUint64List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Sint32:
                    builder.AppendLine($"Schema_GetSint32List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Sint64:
                    builder.AppendLine($"Schema_GetSint64List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Fixed32:
                    builder.AppendLine($"Schema_GetFixed32List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Fixed64:
                    builder.AppendLine($"Schema_GetFixed64List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Sfixed32:
                    builder.AppendLine($"Schema_GetSfixed32List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Sfixed64:
                    builder.AppendLine($"Schema_GetSfixed64List({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Bool:
                    builder.AppendLine($"Schema_GetBoolList({schemaObjectName}, {fieldId}, (uint8*) {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Float:
                    builder.AppendLine($"Schema_GetFloatList({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Double:
                    builder.AppendLine($"Schema_GetDoubleList({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.EntityId:
                    builder.AppendLine($"Schema_GetEntityIdList({schemaObjectName}, {fieldId}, {targetObjectName}.GetData());");
                    break;
                case PrimitiveType.Invalid:
                default:
                    throw new InvalidOperationException("Trying to serialize invalid PrimitiveType");
            }
            return builder.ToString().TrimEnd();
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

        private static string GetValueTypeCount(ValueTypeReference type, string schemaObjectName, string fieldId)
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
                    throw new InvalidOperationException("Trying to deserialize invalid ValueTypeReference");
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
