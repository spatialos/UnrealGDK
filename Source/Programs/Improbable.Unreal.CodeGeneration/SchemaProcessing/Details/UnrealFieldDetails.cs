using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Utils;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.Details
{
    public class UnrealFieldDetails
    {
        public readonly string CapitalisedName;
        public readonly string LowercaseName;
        public readonly IUnrealTypeReference TypeReference;

        public UnrealFieldDetails(FieldDefinitionRaw fieldDefinition, IUnrealTypeReference typeReference)
        {
            CapitalisedName = Formatting.SnakeCaseToCapitalisedCamelCase(fieldDefinition.name);
            LowercaseName = fieldDefinition.name;
            TypeReference = typeReference;
        }

        public bool IsMap()
        {
            return TypeReference.ReferenceType == ReferenceType.Map;
        }

        public bool IsList()
        {
            return TypeReference.ReferenceType == ReferenceType.List;
        }

        public bool IsOption()
        {
            return TypeReference.ReferenceType == ReferenceType.Option;
        }

        public bool IsBuiltIn()
        {
            return TypeReference.ReferenceType == ReferenceType.BuiltIn;
        }

        public bool IsEnum()
        {
            return TypeReference.ReferenceType == ReferenceType.Enum;
        }

        public bool IsUObject()
        {
            switch (TypeReference.ReferenceType)
            {
                case ReferenceType.List:
                case ReferenceType.Map:
                case ReferenceType.Option:
                case ReferenceType.UserType:
                    return true;
                default:
                    return false;
            }
        }
    }
}
