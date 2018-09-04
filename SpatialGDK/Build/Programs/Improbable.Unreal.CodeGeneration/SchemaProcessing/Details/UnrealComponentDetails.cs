using System.Collections.Generic;
using System.Linq;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Utils;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.Details
{
    public class UnrealComponentDetails
    {
        public string CapitalisedName;
        public readonly ComponentDefinitionRaw UnderlyingComponentDefinition;
        public readonly string ComponentId;
        public readonly string UnderlyingCapitalisedName;
        public readonly string UnderlyingQualifiedName;
        public readonly string CapitalisedQualifiedName;
        public readonly string CapitalisedDataName;
        public readonly string QualifiedCapitalisedDataName;
        public readonly UnrealPackageDetails UnderlyingPackageDetails;
        public readonly List<UnrealFieldDetails> FieldDetailsList;
        public readonly List<UnrealEventDetails> EventDetailsList;
        public readonly List<UnrealCommandDetails> CommandDetailsList;

        public UnrealComponentDetails(ComponentDefinitionRaw componentDefinition, string captialisedName, List<UnrealFieldDetails> fieldDetails, List<UnrealEventDetails> eventDetails, List<UnrealCommandDetails> commandDetails, UnrealPackageDetails packageDetails)
        {
            CapitalisedName = captialisedName;
            UnderlyingComponentDefinition = componentDefinition;
            ComponentId = string.Format("{0}", componentDefinition.id);
            UnderlyingQualifiedName = Formatting.QualifiedNameToCppQualifiedName(componentDefinition.qualifiedName);
            UnderlyingCapitalisedName = componentDefinition.name;
            CapitalisedQualifiedName = Formatting.QualifiedNameToCapitalisedCamelCase(componentDefinition.qualifiedName);
            CapitalisedDataName = Formatting.QualifiedNameToCppQualifiedName(componentDefinition.dataDefinition.TypeName.Split('.').Last());
            QualifiedCapitalisedDataName = Formatting.QualifiedNameToCppQualifiedName(componentDefinition.dataDefinition.TypeName);

            UnderlyingPackageDetails = packageDetails;
            FieldDetailsList = fieldDetails;
            EventDetailsList = eventDetails;
            CommandDetailsList = commandDetails;
        }
    }
}
