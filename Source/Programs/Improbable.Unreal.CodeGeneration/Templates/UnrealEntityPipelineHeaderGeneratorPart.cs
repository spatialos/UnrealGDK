using System.Collections.Generic;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealEntityPipelineHeaderGenerator
    {
        private readonly List<UnrealComponentDetails> unrealComponents;

        public UnrealEntityPipelineHeaderGenerator(List<UnrealComponentDetails> unrealComponents)
        {
            this.unrealComponents = unrealComponents;
        }
    }
}
