using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealTypeHeaderGenerator
    {
        private readonly UnrealTypeDetails unrealType;

        public UnrealTypeHeaderGenerator(UnrealTypeDetails unrealType)
        {
            this.unrealType = unrealType;
        }
    }
}
