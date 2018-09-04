using System.Collections.Generic;
using System.Linq;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealCommanderHeaderGenerator
    {
        private readonly List<UnrealCommandDetails> commandDetailsList;
        private readonly IEnumerable<string> packageIncludes;
        private readonly HashSet<string> responseTypeNames;

        public UnrealCommanderHeaderGenerator(List<UnrealCommandDetails> commandDetailsList, IEnumerable<string> packageIncludes)
        {
            this.commandDetailsList = commandDetailsList;
            this.packageIncludes = packageIncludes;
            responseTypeNames = new HashSet<string>(commandDetailsList
                                                    .Select(commandDetails => commandDetails.UnrealResponseTypeDetails)
                                                    .Where(responseType => responseType.ReferenceType == ReferenceType.UserType)
                                                    .Select(responseType => responseType.UClassName));
        }
    }
}
