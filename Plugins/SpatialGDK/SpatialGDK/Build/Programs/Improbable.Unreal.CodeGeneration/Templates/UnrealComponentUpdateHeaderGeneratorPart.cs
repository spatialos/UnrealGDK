using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealComponentUpdateHeaderGenerator
    {
        private readonly UnrealComponentDetails unrealComponent;

        public UnrealComponentUpdateHeaderGenerator(UnrealComponentDetails unrealComponent)
        {
            this.unrealComponent = unrealComponent;
        }
    }
}
