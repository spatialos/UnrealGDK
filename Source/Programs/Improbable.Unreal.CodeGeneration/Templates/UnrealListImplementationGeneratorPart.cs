using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealListImplementationGenerator
    {
        private readonly UnrealListTypeReference listTypeReference;

        public UnrealListImplementationGenerator(UnrealListTypeReference listTypeReference)
        {
            this.listTypeReference = listTypeReference;
        }
    }
}
