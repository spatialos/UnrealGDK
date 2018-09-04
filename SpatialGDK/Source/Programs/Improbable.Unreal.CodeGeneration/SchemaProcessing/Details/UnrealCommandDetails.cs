using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Utils;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.Details
{
    public class UnrealCommandDetails
    {
        public readonly string CapitalisedName;
        public readonly ComponentDefinitionRaw.CommandDefinitionRaw UnderlyingCommandDefinition;
        public readonly string UnderlyingCapitalisedName;
        public readonly UnrealPackageDetails UnderlyingPackageDetails;
        public readonly string QualifiedOwnerName;
        public readonly string CapitalisedOwnerName;
        public readonly IUnrealTypeReference UnrealRequestTypeDetails;
        public readonly IUnrealTypeReference UnrealResponseTypeDetails;
        public readonly string UnrealCommandDelegateType;
        public readonly string UnrealCommandDelegateName;

        public UnrealCommandDetails(ComponentDefinitionRaw.CommandDefinitionRaw commandDefinition,
                                    string capitalisedName,
                                    string qualifiedOwnerName,
                                    string capitalisedOwnerName,
                                    IUnrealTypeReference requestTypeReference,
                                    IUnrealTypeReference responseTypeReference,
                                    UnrealPackageDetails packageDetails)
        {
            CapitalisedName = capitalisedName;
            UnderlyingCommandDefinition = commandDefinition;
            UnderlyingCapitalisedName = Formatting.SnakeCaseToCapitalisedCamelCase(commandDefinition.name);
            UnderlyingPackageDetails = packageDetails;
            QualifiedOwnerName = qualifiedOwnerName;
            CapitalisedOwnerName = capitalisedOwnerName;
            UnrealRequestTypeDetails = requestTypeReference;
            UnrealResponseTypeDetails = responseTypeReference;
            UnrealCommandDelegateType = string.Format("{0}{1}", responseTypeReference.CapitalisedName, requestTypeReference.CapitalisedName);
            UnrealCommandDelegateName = string.Format("F{0}Command", CapitalisedName);
        }
    }
}
