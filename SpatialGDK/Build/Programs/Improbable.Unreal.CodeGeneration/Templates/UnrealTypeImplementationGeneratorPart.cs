using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealTypeImplementationGenerator
    {
        private readonly UnrealTypeDetails unrealType;

        public UnrealTypeImplementationGenerator(UnrealTypeDetails unrealType)
        {
            this.unrealType = unrealType;
        }
    }
}
