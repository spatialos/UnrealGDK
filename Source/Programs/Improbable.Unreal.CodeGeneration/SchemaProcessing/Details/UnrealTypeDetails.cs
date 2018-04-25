using System.Collections.Generic;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Utils;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.Details
{
    public class UnrealTypeDetails
    {
        public readonly string CapitalisedName;
        public readonly TypeDefinitionRaw UnderlyingTypeDefinition;
        public readonly UnrealPackageDetails UnderlyingPackageDetails;
        public readonly string UnderlyingCapitalisedName;
        public readonly string UnderlyingQualifiedName;
        public readonly string CapitalisedQualifiedName;
        public readonly List<UnrealFieldDetails> FieldDetailsList;
        public bool EventsUsingThisType;

        public UnrealTypeDetails(TypeDefinitionRaw typeDefinition, string capitalisedName, List<UnrealFieldDetails> fieldDetails, UnrealPackageDetails packageDetails)
        {
            CapitalisedName = capitalisedName;
            UnderlyingTypeDefinition = typeDefinition;
            UnderlyingPackageDetails = packageDetails;
            UnderlyingCapitalisedName = typeDefinition.name;
            UnderlyingQualifiedName = Formatting.QualifiedNameToCppQualifiedName(typeDefinition.qualifiedName);
            CapitalisedQualifiedName = Formatting.QualifiedNameToCapitalisedCamelCase(typeDefinition.qualifiedName);
            FieldDetailsList = fieldDetails;
            EventsUsingThisType = false;
        }

        public void SetIsUsedForEvent()
        {
            EventsUsingThisType = true;
        }
    }
}
