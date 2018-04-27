using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealOptionImplementationGenerator
    {
        private readonly UnrealOptionTypeReference optionTypeReference;

        public UnrealOptionImplementationGenerator(UnrealOptionTypeReference optionTypeReference)
        {
            this.optionTypeReference = optionTypeReference;
        }
    }
}
