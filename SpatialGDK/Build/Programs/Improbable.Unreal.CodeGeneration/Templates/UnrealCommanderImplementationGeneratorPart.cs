using System.Collections.Generic;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealCommanderImplementationGenerator
    {
        private readonly List<UnrealCommandDetails> commandDetailsList;

        public UnrealCommanderImplementationGenerator(List<UnrealCommandDetails> commandDetailsList)
        {
            this.commandDetailsList = commandDetailsList;
        }
    }
}
