using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealCommandResponderHeaderGenerator
    {
        private readonly UnrealCommandDetails commandDetails;

        public UnrealCommandResponderHeaderGenerator(UnrealCommandDetails commandDetails)
        {
            this.commandDetails = commandDetails;
        }
    }
}
