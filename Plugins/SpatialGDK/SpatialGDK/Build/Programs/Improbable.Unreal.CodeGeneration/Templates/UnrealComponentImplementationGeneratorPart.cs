using System.Linq;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealComponentImplementationGenerator
    {
        private readonly UnrealComponentDetails unrealComponent;

        public UnrealComponentImplementationGenerator(UnrealComponentDetails unrealComponent)
        {
            this.unrealComponent = unrealComponent;
        }

        public string FillComponentInitializerList()
        {
            var result = ": ";

            var uObjectFields = unrealComponent.FieldDetailsList.Where(field => field.IsUObject()).ToList();
            foreach (var fieldDetails in uObjectFields)
            {
                result += fieldDetails.CapitalisedName + "(nullptr)\r\n, ";
            }

            result += "ComponentUpdater(nullptr)";


            foreach (var eventDetails in unrealComponent.EventDetailsList)
            {
                result += "\r\n, " + eventDetails.CapitalisedName + "Wrapper(nullptr)";
            }

            result += "\r\n, Snapshot(nullptr)";

            return result;
        }
    }
}
