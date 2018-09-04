using System.Collections.Generic;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealCallbackDispatcherHeaderGenerator
    {
        private readonly IEnumerable<UnrealComponentDetails> unrealComponents;
        private readonly IEnumerable<string> packageIncludes;

        public UnrealCallbackDispatcherHeaderGenerator(IEnumerable<UnrealComponentDetails> unrealComponents, IEnumerable<string> packageIncludes)
        {
            this.unrealComponents = unrealComponents;
            this.packageIncludes = packageIncludes;
        }
    }
}
