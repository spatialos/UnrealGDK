using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Utils;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.Details
{
    public class UnrealEnumDetails
    {
        public string CapitalisedName;
        public readonly EnumDefinitionRaw UnderlyingEnumDefinition;
        public readonly UnrealPackageDetails UnderlyingPackageDetails;
        public readonly string UnderlyingCapitalisedName;
        public readonly string UnderlyingQualifiedName;
        public readonly string CapitalisedQualifiedName;

        public UnrealEnumDetails(EnumDefinitionRaw enumDefinition, string capitalisedName, UnrealPackageDetails packageDetails)
        {
            CapitalisedName = capitalisedName;
            UnderlyingEnumDefinition = enumDefinition;
            UnderlyingPackageDetails = packageDetails;
            UnderlyingCapitalisedName = enumDefinition.name;
            UnderlyingQualifiedName = Formatting.QualifiedNameToCppQualifiedName(enumDefinition.qualifiedName);
            CapitalisedQualifiedName = Formatting.QualifiedNameToCapitalisedCamelCase(enumDefinition.qualifiedName);
        }
    }
}
