using System.Collections.Generic;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealEntityTemplateHeaderGenerator
    {
        private readonly List<UnrealComponentDetails> unrealComponents;
        private readonly IEnumerable<string> packageIncludes;

        public UnrealEntityTemplateHeaderGenerator(List<UnrealComponentDetails> unrealComponents, IEnumerable<string> packageIncludes)
        {
            this.unrealComponents = unrealComponents;
            this.packageIncludes = packageIncludes;
        }
    }
}
