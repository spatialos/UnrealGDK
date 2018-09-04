using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealMapImplementationGenerator
    {
        private readonly UnrealMapTypeReference mapType;

        public UnrealMapImplementationGenerator(UnrealMapTypeReference mapType)
        {
            this.mapType = mapType;
        }
    }
}
