using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealCommandResponderImplementationGenerator
    {
        private readonly UnrealCommandDetails commandDetails;

        public UnrealCommandResponderImplementationGenerator(UnrealCommandDetails commandDetails)
        {
            this.commandDetails = commandDetails;
        }
    }
}
