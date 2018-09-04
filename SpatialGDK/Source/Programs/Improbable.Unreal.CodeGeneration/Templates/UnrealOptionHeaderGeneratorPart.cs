using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealOptionHeaderGenerator
    {
        private readonly UnrealOptionTypeReference optionTypeReference;

        public UnrealOptionHeaderGenerator(UnrealOptionTypeReference optionTypeReference)
        {
            this.optionTypeReference = optionTypeReference;
        }
    }
}
