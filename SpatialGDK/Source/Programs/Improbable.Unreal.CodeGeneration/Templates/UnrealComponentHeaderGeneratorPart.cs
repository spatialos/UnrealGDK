using System.Collections.Generic;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealComponentHeaderGenerator
    {
        private readonly UnrealComponentDetails unrealComponent;

        public UnrealComponentHeaderGenerator(UnrealComponentDetails unrealComponent)
        {
            this.unrealComponent = unrealComponent;
        }

        // Schema compiler does not generate default constructors for user types, so we need to forward to their copy constructors via initializer list	
        public string FillSnapshotInitializerList()
        {
            var result = "";
            var userTypeCopyConstructors = new List<string>();
            foreach (var fieldDetails in unrealComponent.FieldDetailsList)
            {
                if (fieldDetails.TypeReference.ReferenceType == ReferenceType.UserType)
                {
                    userTypeCopyConstructors.Add(fieldDetails.CapitalisedName +
                                                 "(" + fieldDetails.TypeReference.SnapshotType + "::Create())");
                }
            }

            if (userTypeCopyConstructors.Count > 0)
            {
                result += "\r\n\t: ";
            }

            for (int i = 0; i < userTypeCopyConstructors.Count; ++i)
            {
                result += userTypeCopyConstructors[i];
                if (i != userTypeCopyConstructors.Count - 1)
                {
                    // Unless this is the last line of our initializer list, we need to move to the next line and add ","
                    result += "\r\n\t, ";
                }
            }

            return result;
        }
    }
}
