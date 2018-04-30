using System;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Utils;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.Details
{
    public class UnrealEventDetails
    {
        public readonly string CapitalisedName;
        public readonly string LowercaseName;
        public readonly IUnrealTypeReference EventTypeReference;

        public UnrealEventDetails(ComponentDefinitionRaw.EventDefinitionRaw eventDefinition, IUnrealTypeReference typeReference)
        {
            CapitalisedName = Formatting.SnakeCaseToCapitalisedCamelCase(eventDefinition.name);
            LowercaseName = eventDefinition.name;
            EventTypeReference = typeReference;
        }

        public int NumTypeFields()
        {
            if (EventTypeReference.ReferenceType != ReferenceType.UserType)
            {
                // Note that primitive types are not allowed as event type by the schema compiler,
                // so we should never get here.
                throw new Exception("Events that are not user types are not allowed.");
            }

            var UnrealType = ((UnrealUserTypeReference) EventTypeReference).TypeDetails;
            return UnrealType.FieldDetailsList.Count;
        }
    }
}
