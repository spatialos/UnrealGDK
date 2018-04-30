using System.Collections.Generic;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealEntityPipelineImplementationGenerator
    {
        private readonly List<UnrealComponentDetails> unrealComponents;

        public UnrealEntityPipelineImplementationGenerator(List<UnrealComponentDetails> unrealComponents)
        {
            this.unrealComponents = unrealComponents;
        }
    }
}
