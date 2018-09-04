using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealEnumHeaderGenerator
    {
        private readonly UnrealEnumDetails unrealEnum;

        public UnrealEnumHeaderGenerator(UnrealEnumDetails unrealEnum)
        {
            this.unrealEnum = unrealEnum;
        }
    }
}
