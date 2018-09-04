using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealListHeaderGenerator
    {
        private readonly UnrealListTypeReference listTypeReference;

        public UnrealListHeaderGenerator(UnrealListTypeReference listTypeReference)
        {
            this.listTypeReference = listTypeReference;
        }
    }
}
