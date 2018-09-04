using System.Collections.Generic;
using System.IO;
using Mono.Options;

namespace Improbable.Unreal.CodeGeneration
{
    /// <summary>
    ///     Runtime options for the CodeGenerator.
    /// </summary>
    public class CodeGeneratorOptions
    {
        public static CodeGeneratorOptions ParseArguments(ICollection<string> args)
        {
            var options = new CodeGeneratorOptions();
            var optionSet = new OptionSet
            {
                { "json-dir=", "REQUIRED: the directory containing the json representation of your schema", j => options.JsonDirectory = j },
                { "output-dir=", "the directory to output generated Unreal classes", u => options.OutputDir = u },
                { "h|help", "show help", h => options.ShouldShowHelp = h != null },
                { "package=", "generate only classes in a specific package", p => options.Package = p },
            };

            optionSet.Parse(args);

            using (var sw = new StringWriter())
            {
                optionSet.WriteOptionDescriptions(sw);
                options.HelpText = sw.ToString();
            }

            return options;
        }

        public string JsonDirectory { get; private set; }

        public string Package { get; private set; }

        public string OutputDir { get; private set; }

        public bool ShouldShowHelp { get; private set; }

        public string HelpText { get; private set; }
    }
}
