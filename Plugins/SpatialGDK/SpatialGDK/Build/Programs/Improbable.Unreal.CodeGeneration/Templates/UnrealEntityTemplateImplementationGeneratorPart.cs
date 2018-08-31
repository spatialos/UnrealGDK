using System.Collections.Generic;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealEntityTemplateImplementationGenerator
    {
        private readonly List<UnrealComponentDetails> unrealComponents;

        public UnrealEntityTemplateImplementationGenerator(List<UnrealComponentDetails> unrealComponents)
        {
            this.unrealComponents = unrealComponents;
        }
    }
}
