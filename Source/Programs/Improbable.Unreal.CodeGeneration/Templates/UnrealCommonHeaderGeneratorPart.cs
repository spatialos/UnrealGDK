using System.Collections.Generic;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration
{
    public partial class UnrealCommonHeaderGenerator
    {
        private readonly List<UnrealComponentDetails> unrealComponents;

        public UnrealCommonHeaderGenerator(List<UnrealComponentDetails> unrealComponents)
        {
            this.unrealComponents = unrealComponents;
        }

        public string DefineComponentsType()
        {
            var result = "using Components = worker::Components< ";
            for (int i = 0; i < unrealComponents.Count; ++i)
            {
                result += unrealComponents[i].UnderlyingQualifiedName;
                if (i != unrealComponents.Count - 1)
                {
                    // Unless this is the last template parameter, add "," and move to the next line
                    result += ",\r\n\t\t";
                }
            }

            result += " >;";

            return result;
        }
    }
}
