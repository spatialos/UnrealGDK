using System.Collections.Generic;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealCallbackDispatcherImplementationGenerator
    {
        private readonly IEnumerable<UnrealComponentDetails> unrealComponents;

        public UnrealCallbackDispatcherImplementationGenerator(IEnumerable<UnrealComponentDetails> unrealComponents)
        {
            this.unrealComponents = unrealComponents;
        }
    }
}
