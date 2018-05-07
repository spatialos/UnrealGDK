using System.Linq;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealComponentUpdateImplementationGenerator
    {
        private readonly UnrealComponentDetails unrealComponent;

        public UnrealComponentUpdateImplementationGenerator(UnrealComponentDetails unrealComponent)
        {
            this.unrealComponent = unrealComponent;
        }

        public string FillComponentUpdateInitializerList()
        {
            var result = "";

            var uObjectFields = unrealComponent.FieldDetailsList.Where(field => field.IsUObject()).ToList();
            if (uObjectFields.Count > 0)
            {
                result += "\r\n: ";
            }

            for (var i = 0; i < uObjectFields.Count; ++i)
            {
                result += uObjectFields[i].CapitalisedName + "(nullptr)";
                if (i < uObjectFields.Count - 1)
                {
                    // Unless this is the last line of our initializer list, we need to move to the next line and add ","
                    result += "\r\n, ";
                }
            }

            return result;
        }
    }
}
