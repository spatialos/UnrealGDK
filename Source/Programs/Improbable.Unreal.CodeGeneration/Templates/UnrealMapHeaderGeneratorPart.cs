using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealMapHeaderGenerator
    {
        private readonly UnrealMapTypeReference mapTypeReference;

        public UnrealMapHeaderGenerator(UnrealMapTypeReference mapTypeReference)
        {
            this.mapTypeReference = mapTypeReference;
        }
    }
}
